meta:
  id: fdx
  file-extension: fdx
  bit-endian: le
  
seq:
  - id: magic
    contents:
      - 0x00
  - id: fdx
    type: package
    #terminator: 0x41
    #consume: false
    #repeat: eos
    #eos-error: false
    repeat: expr
    repeat-expr: 100

enums:
  bytetype:
    0: data
    1: header
    
types:
  package:
    seq:
      - id: abyte
        type: dualbyte
        repeat: until
        repeat-until: _.type == bytetype::header
        
  dualbyte:
    seq:
      - id: payload
        size: 1
        type:
          switch-on: type
          cases:
            'bytetype::header': unknown
            'bytetype::data': unknown
      - id: type
        type: b8
        enum: bytetype
      
        
      #- id: packagebit
      #  type: b1
      #  enum: packagetype
      #- id: databytes
      #  if: packagebit == packagetype::data
      #  type: 
      #    switch-on: packagbit
      #    cases:
      #      _: unknown
    enums:
      datatype:
        0x41: package41
        
  headerbyte:
    seq:
      - id: type
        type: b1
      - id: payload
        type: b7
      #- id: payload
      #  type:
      #    switch-on: type
      #    cases:
      #      'packagetype::data': unknown
      #      'packagetype::sender': unknown
    enums:
      packagetype:
        0: data
        1: sender
  datatype:
    seq:
      - id: data
        size: 1
  unknown:
    seq:
      - id: somebytes
        size-eos: true
  dummy: {}
  
  
