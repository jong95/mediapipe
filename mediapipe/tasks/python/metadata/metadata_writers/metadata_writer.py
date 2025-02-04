# Copyright 2022 The MediaPipe Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
"""Generic metadata writer."""

import collections
import dataclasses
import os
import tempfile
from typing import List, Optional, Tuple

import flatbuffers
from mediapipe.tasks.metadata import metadata_schema_py_generated as _metadata_fb
from mediapipe.tasks.python.metadata import metadata as _metadata
from mediapipe.tasks.python.metadata.metadata_writers import metadata_info
from mediapipe.tasks.python.metadata.metadata_writers import writer_utils

_INPUT_IMAGE_NAME = 'image'
_INPUT_IMAGE_DESCRIPTION = 'Input image to be processed.'


@dataclasses.dataclass
class CalibrationParameter:
  """Parameters for score calibration [1].

  Score calibration is performed on an output tensor through sigmoid functions.
  One of the main purposes of score calibration is to make scores across classes
  comparable, so that a common threshold can be used for all output classes.

  For each index in the output tensor, this applies:
    * `f(x) = scale / (1 + e^-(slope * g(x) + offset))` if `x > min_score` or if
      no `min_score` has been specified.
    * `f(x) = default_score` otherwise or if no scale, slope and offset have
      been specified.

  [1]:
    https://github.com/google/mediapipe/blob/f8af41b1eb49ff4bdad756ff19d1d36f486be614/mediapipe/tasks/metadata/metadata_schema.fbs#L434
  """
  scale: float
  slope: float
  offset: float
  min_score: Optional[float] = None


@dataclasses.dataclass
class LabelItem:
  """Label item for labels per locale.

  Attributes:
    filename: The file name to save the labels.
    names: A list of label names.
    locale: The specified locale for labels.
  """
  filename: str
  names: List[str]
  locale: Optional[str] = None


class Labels(object):
  """Simple container holding classification labels of a particular tensor.

  Example usage:
    # The first added label list can be used as category names as needed.
    labels = Labels()
      .add(['/m/011l78', '/m/031d23'])
      .add(['cat', 'dog], 'en')
      .add(['chat', 'chien], 'fr')
  """

  def __init__(self) -> None:
    self._labels = []  # [LabelItem]

  @property
  def labels(self) -> List[LabelItem]:
    return self._labels

  def add(self,
          labels: List[str],
          locale: Optional[str] = None,
          exported_filename: Optional[str] = None) -> 'Labels':
    """Adds labels in the container.

    Args:
      labels: A list of label names, e.g. ['apple', 'pear', 'banana'].
      locale: The specified locale for labels.
      exported_filename: The file name to export the labels. If not set,
        filename defaults to 'labels.txt'.

    Returns:
      The Labels instance, can be used for chained operation.
    """
    if not labels:
      raise ValueError('The list of labels is empty.')

    # Prepare the new item to be inserted
    if not exported_filename:
      exported_filename = 'labels'
      if locale:
        exported_filename += f'_{locale}'
      exported_filename += '.txt'
    item = LabelItem(filename=exported_filename, names=labels, locale=locale)

    # Insert the new element at the end of the list
    self._labels.append(item)
    return self

  def add_from_file(self,
                    label_filepath: str,
                    locale: Optional[str] = None,
                    exported_filename: Optional[str] = None) -> 'Labels':
    """Adds a label file in the container.

    Args:
      label_filepath: File path to read labels. Each line is a label name in the
        file.
      locale: The specified locale for labels.
      exported_filename: The file name to export the labels. If not set,
        filename defaults to 'labels.txt'.

    Returns:
      The Labels instance, can be used for chained operation.
    """

    with open(label_filepath, 'r') as f:
      labels = f.read().split('\n')
      return self.add(labels, locale, exported_filename)


class ScoreCalibration:
  """Simple container holding score calibration related parameters."""

  # A shortcut to avoid client side code importing _metadata_fb
  transformation_types = _metadata_fb.ScoreTransformationType

  def __init__(self,
               transformation_type: _metadata_fb.ScoreTransformationType,
               parameters: List[CalibrationParameter],
               default_score: int = 0):
    self.transformation_type = transformation_type
    self.parameters = parameters
    self.default_score = default_score


def _fill_default_tensor_names(
    tensor_metadata: List[_metadata_fb.TensorMetadataT],
    tensor_names_from_model: List[str]):
  """Fills the default tensor names."""
  # If tensor name in metadata is empty, default to the tensor name saved in
  # the model.
  for metadata, name in zip(tensor_metadata, tensor_names_from_model):
    metadata.name = metadata.name or name


def _pair_tensor_metadata(
    tensor_md: List[metadata_info.TensorMd],
    tensor_names_from_model: List[str]) -> List[metadata_info.TensorMd]:
  """Pairs tensor_md according to the tensor names from the model."""
  tensor_names_from_arg = [
      md.tensor_name for md in tensor_md or [] if md.tensor_name is not None
  ]
  if not tensor_names_from_arg:
    return tensor_md

  if collections.Counter(tensor_names_from_arg) != collections.Counter(
      tensor_names_from_model):
    raise ValueError(
        'The tensor names from arguments ({}) do not match the tensor names'
        ' read from the model ({}).'.format(tensor_names_from_arg,
                                            tensor_names_from_model))
  pairs_tensor_md = []
  name_md_dict = dict(zip(tensor_names_from_arg, tensor_md))
  for name in tensor_names_from_model:
    pairs_tensor_md.append(name_md_dict[name])
  return pairs_tensor_md


def _create_metadata_buffer(
    model_buffer: bytearray,
    general_md: Optional[metadata_info.GeneralMd] = None,
    input_md: Optional[List[metadata_info.TensorMd]] = None,
    output_md: Optional[List[metadata_info.TensorMd]] = None) -> bytearray:
  """Creates a buffer of the metadata.

  Args:
    model_buffer: valid buffer of the model file.
    general_md: general information about the model.
    input_md: metadata information of the input tensors.
    output_md: metadata information of the output tensors.

  Returns:
    A buffer of the metadata.

  Raises:
    ValueError: if the tensor names from `input_md` and `output_md` do not
    match the tensor names read from the model.
  """
  # Create input metadata from `input_md`.
  if input_md:
    input_md = _pair_tensor_metadata(
        input_md, writer_utils.get_input_tensor_names(model_buffer))
    input_metadata = [m.create_metadata() for m in input_md]
  else:
    num_input_tensors = writer_utils.get_subgraph(model_buffer).InputsLength()
    input_metadata = [_metadata_fb.TensorMetadataT()] * num_input_tensors

  _fill_default_tensor_names(input_metadata,
                             writer_utils.get_input_tensor_names(model_buffer))

  # Create output metadata from `output_md`.
  if output_md:
    output_md = _pair_tensor_metadata(
        output_md, writer_utils.get_output_tensor_names(model_buffer))
    output_metadata = [m.create_metadata() for m in output_md]
  else:
    num_output_tensors = writer_utils.get_subgraph(model_buffer).OutputsLength()
    output_metadata = [_metadata_fb.TensorMetadataT()] * num_output_tensors
  _fill_default_tensor_names(output_metadata,
                             writer_utils.get_output_tensor_names(model_buffer))

  # Create the subgraph metadata.
  subgraph_metadata = _metadata_fb.SubGraphMetadataT()
  subgraph_metadata.inputTensorMetadata = input_metadata
  subgraph_metadata.outputTensorMetadata = output_metadata

  # Create the whole model metadata.
  if general_md is None:
    general_md = metadata_info.GeneralMd()
  model_metadata = general_md.create_metadata()
  model_metadata.subgraphMetadata = [subgraph_metadata]

  # Get the metadata flatbuffer.
  b = flatbuffers.Builder(0)
  b.Finish(
      model_metadata.Pack(b),
      _metadata.MetadataPopulator.METADATA_FILE_IDENTIFIER)
  return b.Output()


class MetadataWriter(object):
  """Generic Metadata writer.

  Example usage:

  For an example model which requires two inputs: image and general feature
  inputs, and generates one output: classification.

  with open(model_path, 'rb') as f:
    writer = MetadataWriter.create(f.read())
    model_content, metadata_json_content = writer
        .add_genernal_info('model_name', 'model description')
        .add_image_input()
        .add_feature_input()
        .add_classification_output(Labels().add(['A', 'B']))
        .populate()
  """

  @classmethod
  def create(cls, model_buffer: bytearray) -> 'MetadataWriter':
    return cls(model_buffer)

  def __init__(self, model_buffer: bytearray) -> None:
    self._model_buffer = model_buffer
    self._general_md = None
    self._input_mds = []
    self._output_mds = []
    self._associated_files = []
    self._temp_folder = tempfile.TemporaryDirectory()

  def __del__(self):
    if os.path.exists(self._temp_folder.name):
      self._temp_folder.cleanup()

  def add_genernal_info(
      self,
      model_name: str,
      model_description: Optional[str] = None) -> 'MetadataWriter':
    """Adds a genernal info metadata for the general metadata informantion."""
    # Will overwrite the previous `self._general_md` if exists.
    self._general_md = metadata_info.GeneralMd(
        name=model_name, description=model_description)
    return self

  color_space_types = _metadata_fb.ColorSpaceType

  def add_feature_input(self,
                        name: Optional[str] = None,
                        description: Optional[str] = None) -> 'MetadataWriter':
    """Adds an input tensor metadata for the general basic feature input."""
    input_md = metadata_info.TensorMd(name=name, description=description)
    self._input_mds.append(input_md)
    return self

  def add_image_input(
      self,
      norm_mean: List[float],
      norm_std: List[float],
      color_space_type: Optional[int] = _metadata_fb.ColorSpaceType.RGB,
      name: str = _INPUT_IMAGE_NAME,
      description: str = _INPUT_IMAGE_DESCRIPTION) -> 'MetadataWriter':
    """Adds an input image metadata for the image input.

    Args:
      norm_mean: The mean value used to normalize each input channel. If there
        is only one element in the list, its value will be broadcasted to all
        channels. Also note that norm_mean and norm_std should have the same
        number of elements. [1]
      norm_std: The std value used to normalize each input channel. If there is
        only one element in the list, its value will be broadcasted to all
        channels. [1]
      color_space_type: The color space type of the input image. [2]
      name: Name of the input tensor.
      description: Description of the input tensor.

    Returns:
      The MetaWriter instance, can be used for chained operation.

    [1]:
      https://github.com/google/mediapipe/blob/f8af41b1eb49ff4bdad756ff19d1d36f486be614/mediapipe/tasks/metadata/metadata_schema.fbs#L389
    [2]:
      https://github.com/google/mediapipe/blob/f8af41b1eb49ff4bdad756ff19d1d36f486be614/mediapipe/tasks/metadata/metadata_schema.fbs#L198
    """
    input_md = metadata_info.InputImageTensorMd(
        name=name,
        description=description,
        norm_mean=norm_mean,
        norm_std=norm_std,
        color_space_type=color_space_type,
        tensor_type=self._input_tensor_type(len(self._input_mds)))

    self._input_mds.append(input_md)
    return self

  _OUTPUT_CLASSIFICATION_NAME = 'score'
  _OUTPUT_CLASSIFICATION_DESCRIPTION = 'Score of the labels respectively'

  def add_classification_output(
      self,
      labels: Optional[Labels] = None,
      score_calibration: Optional[ScoreCalibration] = None,
      name: str = _OUTPUT_CLASSIFICATION_NAME,
      description: str = _OUTPUT_CLASSIFICATION_DESCRIPTION
  ) -> 'MetadataWriter':
    """Add a classification head metadata for classification output tensor.

    Example usage:
      writer.add_classification_output(
        Labels()
          .add(['/m/011l78', '/m/031d23'])
          .add(['cat', 'dog], 'en')
          .add(['chat', 'chien], 'fr')
          )

    Args:
      labels: an instance of Labels helper class.
      score_calibration: an instance of ScoreCalibration helper class.
      name: Metadata name of the tensor. Note that this is different from tensor
        name in the flatbuffer.
      description: human readable description of what the output is.

    Returns:
      The current Writer instance to allow chained operation.
    """
    calibration_md = None
    if score_calibration:
      calibration_md = metadata_info.ScoreCalibrationMd(
          score_transformation_type=score_calibration.transformation_type,
          default_score=score_calibration.default_score,
          file_path=self._export_calibration_file('score_calibration.txt',
                                                  score_calibration.parameters))

    label_files = None
    if labels:
      label_files = []
      for item in labels.labels:
        label_files.append(
            metadata_info.LabelFileMd(
                self._export_labels(item.filename, item.names),
                locale=item.locale))

    output_md = metadata_info.ClassificationTensorMd(
        name=name,
        description=description,
        label_files=label_files,
        tensor_type=self._output_tensor_type(len(self._output_mds)),
        score_calibration_md=calibration_md,
    )
    self._output_mds.append(output_md)
    return self

  def add_feature_output(self,
                         name: Optional[str] = None,
                         description: Optional[str] = None) -> 'MetadataWriter':
    """Adds an output tensor metadata for the general basic feature output."""
    output_md = metadata_info.TensorMd(name=name, description=description)
    self._output_mds.append(output_md)
    return self

  def populate(self) -> Tuple[bytearray, str]:
    """Populates metadata into the TFLite file.

    Note that only the output tflite is used for deployment. The output JSON
    content is used to interpret the metadata content.

    Returns:
      A tuple of (model_with_metadata_in_bytes, metdata_json_content)
    """
    # Populates metadata and associated files into TFLite model buffer.
    populator = _metadata.MetadataPopulator.with_model_buffer(
        self._model_buffer)
    metadata_buffer = _create_metadata_buffer(
        model_buffer=self._model_buffer,
        general_md=self._general_md,
        input_md=self._input_mds,
        output_md=self._output_mds)
    populator.load_metadata_buffer(metadata_buffer)
    if self._associated_files:
      populator.load_associated_files(self._associated_files)
    populator.populate()
    tflite_content = populator.get_model_buffer()

    displayer = _metadata.MetadataDisplayer.with_model_buffer(tflite_content)
    metadata_json_content = displayer.get_metadata_json()

    return tflite_content, metadata_json_content

  def _input_tensor_type(self, idx):
    return writer_utils.get_input_tensor_types(self._model_buffer)[idx]

  def _output_tensor_type(self, idx):
    return writer_utils.get_output_tensor_types(self._model_buffer)[idx]

  def _export_labels(self, filename: str, index_to_label: List[str]) -> str:
    filepath = os.path.join(self._temp_folder.name, filename)
    with open(filepath, 'w') as f:
      f.write('\n'.join(index_to_label))
    self._associated_files.append(filepath)
    return filepath

  def _export_calibration_file(self, filename: str,
                               calibrations: List[CalibrationParameter]) -> str:
    """Stores calibration parameters in a csv file."""
    filepath = os.path.join(self._temp_folder.name, filename)
    with open(filepath, 'w') as f:
      for idx, item in enumerate(calibrations):
        if idx != 0:
          f.write('\n')
        if item:
          if item.scale is None or item.slope is None or item.offset is None:
            raise ValueError('scale, slope and offset values can not be set to '
                             'None.')
          elif item.min_score is not None:
            f.write(f'{item.scale},{item.slope},{item.offset},{item.min_score}')
          else:
            f.write(f'{item.scale},{item.slope},{item.offset}')

          self._associated_files.append(filepath)
    return filepath
