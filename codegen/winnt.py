from time import time_ns
from dataclasses import dataclass
from enum import IntEnum
from ctypes import c_uint8, c_uint16, c_uint32, c_uint64, _SimpleCData, sizeof 
from os import getenv

DEBUG = int(getenv('DEBUG', "1"))

# 
# Basic types 
# 

class CArrayType(type): 

  def __mul__(self, other): 
    assert type(other) == int, 'Can only multiply with a integer'
    assert other > 0, f'Must be above zero, got {other}'

    _self = self
    def init(self, *args): 
      assert len(args) == other, f'Incorrect number of arguments expected {other}, but got {len(args)}'
      for i, arg in enumerate(args): 
        assert isinstance(arg, _self) or type(arg) == int, f'Bad argument {i+1} type {type(arg)} expected {_self}'
      self._internal_array = [arg if isinstance(arg, _self) else _self._value_type(arg) for arg in args]

    def setitem(self, k, v): 
      if type(v) == _self:
        self._internal_array[k] = v
      elif type(v) == int:
        self._internal_array[k] = _self(v)
      else: 
        raise TypeError(f'Item with an unexpected type {type(v)} given, expected {_self}')
    
    def _set(self, obj, value): 
      if len(value) != len(self._internal_array):
        raise ValueError(f'Expected value with length {other}, got {len(value)}')

      if isinstance(type(value), CArrayType): 
        self._internal_array = value._internal_array
        setattr(obj, self._name, value)
        return
      elif type(value) == str:
        self._internal_array = [c_uint8(ord(v)) for v in value]
      elif type(value) == list:
        self._internal_array = value
      else: 
        raise ValueError(f'Got unknown value {type(value)}')
      setattr(obj, self._name, self)

    def size(self): 
      single_value = self._internal_array[0]
      if isinstance(single_value, Struct):
        return single_value.size()  * other
      elif isinstance(single_value, _SimpleCData):
        return sizeof(single_value) * other
      elif isinstance(single_value, CType):
        return sizeof(single_value._value_type) * other
      return 0

    def get_bytes(self): 
      bytes_stream = bytearray()
      for v in self.__dict__.values():
        if isinstance(v, _SimpleCData): 
          bytes_stream += bytes(v)
        elif type(v) == list: 
          for val in v:
            if isinstance(val, _SimpleCData): bytes_stream += bytes(val)
            else: bytes_stream += val.get_bytes()
        else: 
          if not type(v) == str: # I unimportant string values 
            print(bytes_stream)
            raise Exception('TODO: Fix whatever is in this path')
      return bytes_stream

    def set_name(self, owner, name): self._name = "_" + name

    cls_dict = { 
      '__init__' :    init, 
      '__len__'  :    lambda self: len(self._internal_array),
      '__getitem__' : lambda self, k: self._internal_array[k], 
      '__setitem__' : setitem,
      '__set__'     : _set,
      '__set_name__' : set_name,
      'size' :        size, 
      'get_bytes' :   get_bytes 
    }

    cls_name = str(self).split('.')[1].split("'")[0]
    return type(f'{cls_name}_Array_{other}', (Struct, ), cls_dict)

class CType(metaclass=CArrayType):
  _value_type=int
  def __init__(self, value=0):
    self._value = value
      
  def get_bytes(self):
    if type(self._value) == str: result = ord(self._value)
    else: result = self._value
    if isinstance(result, _SimpleCData) or isinstance(self._value_type(), _SimpleCData):
      result = bytes(self._value_type(result))
    else: 
      print('YOOOOOOOOOOOOOOO HOW DID THIS GET TO HERE?!?!? Probably a bug you did not see comming :)\n', result, self._value, type(self._value), self._value_type)
    return result

  def __set_name__(self, owner, name):
    self._name = "_" + name
      
  def __get__(self, obj, type):
    if obj is None:
      return self._value
    return getattr(obj, self._name, self._value)
  
  def __set__(self, obj, value):
    if not (type(value) == int or isinstance(value, IntEnum) or isinstance(value, _SimpleCData)): 
      raise AttributeError(f'Expected value of type {self._value_type}, got {type(value)}')
    setattr(obj, self._name, self._value_type(value))
  
  def __repr__(self):
    return str(self._value)
      
class u8(CType):  _value_type=c_uint8
class u16(CType): _value_type=c_uint16
class u32(CType): _value_type=c_uint32
class u64(CType): _value_type=c_uint64

class Struct(metaclass=CArrayType):
  def size(self): 
    arr = []
    for v in self.__dict__.values():
      if isinstance(v, _SimpleCData): arr.append(sizeof(v))
      elif isinstance(v, Struct): arr.append(v.size())
      else: raise TypeError(f'No known size for type {type(v)}')
    return sum(arr)

  def get_bytes(self): 
    bytes_stream = bytearray()
    for v in self.__dict__.values():
      if isinstance(v, _SimpleCData): bytes_stream += bytes(v)
      else: bytes_stream += v.get_bytes()
    return bytes_stream

# 
# Header definitions  
# 

# From: https://tech-zealots.com/malware-analysis/pe-portable-executable-structure-malware-analysis-part-2/ 
# Needed signiture for windows to recognize as binary format it can execute
# note that 'F8 00 00 00' here is the address it will jump to for the IMAGE_NT_SIGNITURE
# and you must have this address begin at the address 0x3c which here it does.
# Currently it is set to be at address 0x000000f8 because of the IMAGE_DOS_STUB
# but change it accordingly to your stub (you don't need to care of any of these values
# if you don't plan to have a stub aside from the first two bytes that spell `MZ` and the 
# address from which you will have the PE_SIGNITURE from)
IMAGE_DOS_SIGNITURE = bytes.fromhex("" + \
  "4D5A90000300000004000000FFFF0000" + \
  "B8000000000000004000000000000000" + \
  "00000000000000000000000000000000" + \
  "000000000000000000000000F8000000")

# https://osandamalith.com/2020/07/19/exploring-the-ms-dos-stub/
# a stub for when you are trying to run in dos mode as we don't support it  
# it contains the instructions for a dos program that prints 'This program 
# cannot be run in DOS mode'. This specific stub was robbed from a random 
# executable I had laying around, you don't need it but it is cool.
IMAGE_DOS_SUB = bytes.fromhex("" + \
  "0E1FBA0E00B409CD21B8014CCD215468" + \
  "69732070726F6772616D2063616E6E6F" + \
  "742062652072756E20696E20444F5320" + \
  "6D6F64652E0D0D0A2400000000000000" + \
  "860708F2C26666A1C26666A1C26666A1" + \
  "D60D62A0C96666A1D60D65A0C56666A1" + \
  "D60D63A0526666A1A01E63A0E56666A1" + \
  "A01E62A0D26666A1A01E65A0CB6666A1" + \
  "D60D67A0CB6666A1C26667A1B16666A1" + \
  "A31C63A0C36666A1A31C64A0C36666A1" + \
  "52696368C26666A10000000000000000" + \
  "0000000000000000")

# NOTE: This has to be dword aligned
PE_SIGNITURE = b'PE\0\0'

# 
# File header format
# 

class Machine_Flags(IntEnum):
  IMAGE_FILE_MACHINE_UNKNOWN     = 0x0
  IMAGE_FILE_MACHINE_AM33        = 0x1d3
  IMAGE_FILE_MACHINE_AMD64       = 0x8664
  IMAGE_FILE_MACHINE_ARM         = 0x1c0
  IMAGE_FILE_MACHINE_ARM64       = 0xaa64
  IMAGE_FILE_MACHINE_ARMNT       = 0x1c4
  IMAGE_FILE_MACHINE_EBC         = 0xebc
  IMAGE_FILE_MACHINE_I386        = 0x14c 
  IMAGE_FILE_MACHINE_IA64        = 0x200 
  IMAGE_FILE_MACHINE_LOONGARCH32 = 0x6232
  IMAGE_FILE_MACHINE_LOONGARCH64 = 0x6264
  IMAGE_FILE_MACHINE_M32R        = 0x9041
  IMAGE_FILE_MACHINE_MIPS16      = 0x266
  IMAGE_FILE_MACHINE_MIPSFPU     = 0x366
  IMAGE_FILE_MACHINE_MIPSFPU16   = 0x466
  IMAGE_FILE_MACHINE_POWERPC     = 0x1f0
  IMAGE_FILE_MACHINE_POWERPCFP   = 0x1f1
  IMAGE_FILE_MACHINE_R4000       = 0x166
  IMAGE_FILE_MACHINE_RISCV32     = 0x5032
  IMAGE_FILE_MACHINE_RISCV64     = 0x5064
  IMAGE_FILE_MACHINE_RISCV128    = 0x5128
  IMAGE_FILE_MACHINE_SH3         = 0x1a2
  IMAGE_FILE_MACHINE_SH3DSP      = 0x1a3
  IMAGE_FILE_MACHINE_SH4         = 0x1a6
  IMAGE_FILE_MACHINE_SH5         = 0x1a8
  IMAGE_FILE_MACHINE_THUMB       = 0x1c2
  IMAGE_FILE_MACHINE_WCEMIPSV2   = 0x169

class Characteristics_Flags(IntEnum):
  # Image File Header Characteristics
  IMAGE_FILE_RELOCS_STRIPPED     = 0x0001 
  IMAGE_FILE_EXECUTABLE_IMAGE    = 0x0002 
  IMAGE_FILE_LINE_NUMS_STRIPPED  = 0x0004 
  IMAGE_FILE_LOCAL_SYMS_STRIPPED = 0x0008 
  IMAGE_FILE_AGGRESSIVE_WS_TRIM  = 0x0010 
  IMAGE_FILE_LARGE_ADDRESS_AWARE = 0x0020 
  IMAGE_FILE_BYTES_REVERSED_LO   = 0x0080 
  IMAGE_FILE_32BIT_MACHINE       = 0x0100 
  IMAGE_FILE_DEBUG_STRIPPED      = 0x0200 
  IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP = 0x0400 
  IMAGE_FILE_NET_RUN_FROM_SWAP   = 0x0800 
  IMAGE_FILE_SYSTEM              = 0x1000 
  IMAGE_FILE_DLL                 = 0x2000 
  IMAGE_FILE_UP_SYSTEM_ONLY      = 0x4000 
  IMAGE_FILE_BYTES_REVERSED_HI   = 0x8000 

  # Sections Header Characteristics
  IMAGE_SCN_TYPE_NO_PAD     = 0x00000008
  IMAGE_SCN_CNT_CODE        = 0x00000020 
  IMAGE_SCN_CNT_INITIALIZED_DATA   = 0x00000040 
  IMAGE_SCN_CNT_UNINITIALIZED_DATA = 0x00000080 
  IMAGE_SCN_LNK_OTHER       = 0x00000100 
  IMAGE_SCN_LNK_INFO        = 0x00000200 
  IMAGE_SCN_LNK_REMOVE      = 0x00000800 
  IMAGE_SCN_LNK_COMDAT      = 0x00001000 
  IMAGE_SCN_GPREL           = 0x00008000 
  IMAGE_SCN_MEM_PURGEABLE   = 0x00020000 
  IMAGE_SCN_MEM_16BIT       = 0x00020000 
  IMAGE_SCN_MEM_LOCKED      = 0x00040000 
  IMAGE_SCN_MEM_PRELOAD     = 0x00080000 
  IMAGE_SCN_ALIGN_1BYTES    = 0x00100000 
  IMAGE_SCN_ALIGN_2BYTES    = 0x00200000 
  IMAGE_SCN_ALIGN_4BYTES    = 0x00300000 
  IMAGE_SCN_ALIGN_8BYTES    = 0x00400000 
  IMAGE_SCN_ALIGN_16BYTES   = 0x00500000 
  IMAGE_SCN_ALIGN_32BYTES   = 0x00600000 
  IMAGE_SCN_ALIGN_64BYTES   = 0x00700000 
  IMAGE_SCN_ALIGN_128BYTES  = 0x00800000 
  IMAGE_SCN_ALIGN_256BYTES  = 0x00900000 
  IMAGE_SCN_ALIGN_512BYTES  = 0x00A00000 
  IMAGE_SCN_ALIGN_1024BYTES = 0x00B00000 
  IMAGE_SCN_ALIGN_2048BYTES = 0x00C00000 
  IMAGE_SCN_ALIGN_4096BYTES = 0x00D00000 
  IMAGE_SCN_ALIGN_8192BYTES = 0x00E00000 
  IMAGE_SCN_LNK_NRELOC_OVFL = 0x01000000 
  IMAGE_SCN_MEM_DISCARDABLE = 0x02000000 
  IMAGE_SCN_MEM_NOT_CACHED  = 0x04000000 
  IMAGE_SCN_MEM_NOT_PAGED   = 0x08000000 
  IMAGE_SCN_MEM_SHARED      = 0x10000000 
  IMAGE_SCN_MEM_EXECUTE     = 0x20000000 
  IMAGE_SCN_MEM_READ        = 0x40000000 
  IMAGE_SCN_MEM_WRITE       = 0x80000000 


@dataclass
class IMAGE_FILE_HEADER(Struct): 
  Machine:          u16 = u16(Machine_Flags.IMAGE_FILE_MACHINE_AMD64)
  NumberOfSections: u16 = u16(2) # .text .data and so on sections
  TimeDateStamp:    u32 = u32(int(time_ns()//1e9))
  PointerToSymbolTable: u32 = u32(0) # probably for COFF object file 
  NumberOfSymbols:      u32 = u32(0) # probably for COFF object file 
  SizeOfOptionalHeader: u16 = u16(0) # NOTE: must for .exe files but not object files 
  Characteristics: u16 = u16(Characteristics_Flags.IMAGE_FILE_EXECUTABLE_IMAGE | 
                              Characteristics_Flags.IMAGE_FILE_LARGE_ADDRESS_AWARE)

IMAGE_SIZEOF_FILE_HEADER = 20
if DEBUG: assert IMAGE_FILE_HEADER().size() == IMAGE_SIZEOF_FILE_HEADER

# 
# Directory format
# 

@dataclass
class IMAGE_DATA_DIRECTORY(Struct):
  VirtualAddress: u32 = u32(0)
  Size: u32 = u32(0)


IMAGE_NUMBEROF_DIRECTORY_ENTRIES = 16

#
# Optional header format
#

class Magic_Flags(IntEnum):
  PE32      = 0x10b 
  ROM       = 0x107
  PE32_PLUS = 0x20b

class Windows_Subsystem_Flags(IntEnum):
  IMAGE_SUBSYSTEM_UNKNOWN         = 0
  IMAGE_SUBSYSTEM_NATIVE          = 1
  IMAGE_SUBSYSTEM_WINDOWS_GUI     = 2
  IMAGE_SUBSYSTEM_WINDOWS_CUI     = 3
  IMAGE_SUBSYSTEM_OS2_CUI         = 5
  IMAGE_SUBSYSTEM_POSIX_CUI       = 7
  IMAGE_SUBSYSTEM_NATIVE_WINDOWS  = 8
  IMAGE_SUBSYSTEM_WINDOWS_CE_GUI  = 9
  IMAGE_SUBSYSTEM_EFI_APPLICATION = 10
  IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  = 11
  IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER       = 12
  IMAGE_SUBSYSTEM_EFI_ROM                  = 13
  IMAGE_SUBSYSTEM_XBOX                     = 14
  IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION = 16

class DllCharacteristics_Flags(IntEnum):
  IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA = 0x0020
  IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE    = 0x0040
  IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY = 0x0080
  IMAGE_DLLCHARACTERISTICS_NX_COMPAT       = 0x0100
  IMAGE_DLLCHARACTERISTICS_NO_ISOLATION    = 0x0200
  IMAGE_DLLCHARACTERISTICS_NO_SEH          = 0x0400
  IMAGE_DLLCHARACTERISTICS_NO_BIND         = 0x0800
  IMAGE_DLLCHARACTERISTICS_APPCONTAINER    = 0x1000
  IMAGE_DLLCHARACTERISTICS_WDM_DRIVER      = 0x2000
  IMAGE_DLLCHARACTERISTICS_GUARD_CF        = 0x4000
  IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE  = 0x8000

# https://blog.kowalczyk.info/articles/pefileformat.html 
# https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#optional-header-standard-fields-image-only

IMAGE_DATA_DIRECTORY_x16 = IMAGE_DATA_DIRECTORY*IMAGE_NUMBEROF_DIRECTORY_ENTRIES
# TODO: Create the 32bit version and check it's correctness 
@dataclass
class IMAGE_OPTIONAL_HEADER64(Struct):
  #  
  # Standard fields.
  # 
  Magic:              u16 = u16(Magic_Flags.PE32_PLUS)
  MajorLinkerVersion: u8 = u8(0) # We are not a linker 
  MinorLinkerVersion: u8 = u8(0) # We are not a linker 

  # NOTE: The following 4 values MUST be replaced with valid values by the user
  SizeOfCode:            u32   = u32(0) # size of .text section  
  SizeOfInitializedData: u32   = u32(0) # size of .data section 
  SizeOfUninitializedData: u32 = u32(0) # size of .bss section 
  AddressOfEntryPoint:     u32 = u32(0) # address of the entry point
  BaseOfCode: u32 = u32(0x1000) # .text 
  # BaseOfData: u32 = u32(0) # .bss only for 32bit
  #
  # NT additional fields.
  # 
  
  ImageBase:        u64 = u64(0x400000)
  SectionAlignment: u32 = u32(0x1000) # Minimum space that can a section can occupy when loaded that is, 
                                      # sections are aligned on SectionAlignment boundaries which default 
                                      # to a Page size (usually 4kb )
  FileAlignment:    u32 = u32(0x200)  # .exe alignment on disk
  MajorOperatingSystemVersion: u16 = u16(6)
  MinorOperatingSystemVersion: u16 = u16(0)
  MajorImageVersion: u16 = u16(0) # Ignore this value 
  MinorImageVersion: u16 = u16(0) # Ignore this value 
  MajorSubsystemVersion: u16 = u16(6)
  MinorSubsystemVersion: u16 = u16(0)
  Reserved1:     u32 = u32(0)
  SizeOfImage:   u32 = u32(0) # NOTE: user MUST provide valid value
  SizeOfHeaders: u32 = u32(0) # NOTE: user MUST provide valid value
  CheckSum:  u32 = u32(0) # Supposedly a CRC checksum of the file. As in other Microsoft executable formats, 
                          # this field is ignored and set to 0. The one exception to this rule is for trusted 
                          # services and these EXEs must have a valid checksum.
  Subsystem: u16 = u16(Windows_Subsystem_Flags.IMAGE_SUBSYSTEM_WINDOWS_GUI)
  DllCharacteristics: u16 = u16(0) # TODO: Figure out whether there are any good default values to put here
  SizeOfStackReserve: u64 = u64(0x100000)
  SizeOfStackCommit:  u64 = u64(0x1000)
  SizeOfHeapReserve:  u64 = u64(0x100000)
  SizeOfHeapCommit:   u64 = u64(0x1000)
  LoaderFlags:        u32 = u32(0)
  NumberOfRvaAndSizes:u32 = u32(IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
  DataDirectory: IMAGE_DATA_DIRECTORY_x16 = (IMAGE_DATA_DIRECTORY_x16)(*[IMAGE_DATA_DIRECTORY() for _ in range(16)]) 



IMAGE_SIZEOF_SHORT_NAME = 8
byte_array_x8 = u8*IMAGE_SIZEOF_SHORT_NAME

@dataclass
class IMAGE_SECTION_HEADER(Struct): 
  Name: byte_array_x8 = byte_array_x8(*([0]*8))
  VirtualSize:          u32 = u32(0) # PhysicalAddress 
  VirtualAddress:       u32 = u32(0)
  SizeOfRawData:        u32 = u32(0)
  PointerToRawData:     u32 = u32(0)
  PointerToRelocations: u32 = u32(0)
  PointerToLinenumbers: u32 = u32(0)
  NumberOfRelocations:  u16 = u16(0)
  NumberOfLinenumbers:  u16 = u16(0)
  Characteristics:      u32 = u32(0)

IMAGE_SIZEOF_SECTION_HEADER = 40
if DEBUG: assert IMAGE_SECTION_HEADER().size() == IMAGE_SIZEOF_SECTION_HEADER

@dataclass
class IMAGE_IMPORT_DESCRIPTOR(Struct):
  OriginalFirstThunk: u32 = u32(0) # Pointer to Import Name Table
  TimeDateStamp:      u32 = u32(0) 
  ForwarderChain:     u32 = u32(0)
  Name:               u32 = u32(0) # Pointer to Module name e.g. kernel32.dll
  FirstThunk:         u32 = u32(0) # Pointer to Import Address Table Null-terminated list of 
                                   # pointers On file it is a copy of the INT After loading 
                                   # it points to the imported APIs