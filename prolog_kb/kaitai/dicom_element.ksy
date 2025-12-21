meta:
  id: dicom_element
  title: DICOM Data Element
  endian: le
  license: MIT

doc: |
  DICOM Part 5 Data Element structure for Explicit VR Little Endian
  transfer syntax. Used for parsing individual DICOM data elements.

seq:
  - id: tag
    type: dicom_tag
  - id: vr
    type: value_representation
  - id: value
    size: vr.value_length
    type:
      switch-on: vr.vr_code
      cases:
        '"US"': u2_value
        '"SS"': s2_value
        '"UL"': u4_value
        '"SL"': s4_value
        '"FL"': f4_value
        '"FD"': f8_value
        '"SQ"': sequence_value
        _: raw_value

types:
  dicom_tag:
    doc: DICOM (group, element) tag pair
    seq:
      - id: group
        type: u2
      - id: element
        type: u2

  value_representation:
    doc: Value Representation with length encoding
    seq:
      - id: vr_code
        type: str
        size: 2
        encoding: ASCII
      - id: length_field
        type:
          switch-on: is_long_vr
          cases:
            true: long_length
            false: short_length
    instances:
      is_long_vr:
        value: >-
          vr_code == "OB" or vr_code == "OD" or vr_code == "OF" or
          vr_code == "OL" or vr_code == "OW" or vr_code == "SQ" or
          vr_code == "UC" or vr_code == "UN" or vr_code == "UR" or
          vr_code == "UT"
      value_length:
        value: length_field.length

  short_length:
    seq:
      - id: length
        type: u2

  long_length:
    seq:
      - id: reserved
        type: u2
      - id: length
        type: u4

  u2_value:
    seq:
      - id: value
        type: u2

  s2_value:
    seq:
      - id: value
        type: s2

  u4_value:
    seq:
      - id: value
        type: u4

  s4_value:
    seq:
      - id: value
        type: s4

  f4_value:
    seq:
      - id: value
        type: f4

  f8_value:
    seq:
      - id: value
        type: f8

  sequence_value:
    doc: Sequence of Items (undefined length handled separately)
    seq:
      - id: items
        type: sequence_item
        repeat: until
        repeat-until: _.is_delimiter

  sequence_item:
    seq:
      - id: item_tag
        type: dicom_tag
      - id: item_length
        type: u4
      - id: elements
        type: dicom_element
        repeat: eos
        if: item_length != 0xFFFFFFFF
    instances:
      is_delimiter:
        value: item_tag.group == 0xFFFE and item_tag.element == 0xE00D

  raw_value:
    seq:
      - id: data
        size-eos: true
