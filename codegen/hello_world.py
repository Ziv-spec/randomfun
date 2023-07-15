from winnt import IMAGE_DOS_SIGNITURE, IMAGE_DOS_SUB, PE_SIGNITURE, \
                  IMAGE_FILE_HEADER, IMAGE_OPTIONAL_HEADER64, IMAGE_SECTION_HEADER, \
                  IMAGE_IMPORT_DESCRIPTOR, Characteristics_Flags, u8, u64, DEBUG
import functools
import operator

def align(val, alignment): return (val // alignment + 1) * alignment
def flatten(x): return functools.reduce(operator.add, x, [])
def to_hex(x): return ''.join([hex(v)[2:].rjust(2, '0') for v in bytes(x, 'ascii')])

def gen_empty_import_structure(spec):
  # NOTE: Here we will generate mostly empty structures that will get filled later

  # Generate import descriptors
  # Generate INT - Import Name Table 
  # Generate IMAGE_IMPORT_BY_NAME - Hint: u16, Name: u8*n
  # Generate IAT - Import Address Table 
  # Generate Module Names
  descriptors = (IMAGE_IMPORT_DESCRIPTOR*(len(spec)+1))(*([IMAGE_IMPORT_DESCRIPTOR() for _ in range(len(spec)+1)]))
  int_tables = [(u64*(len(d[1])+1))(*([0]*(len(d[1])+1))) for d in spec]
  image_import_by_name_array = [[bytes('\0'*2 + name + '\0', 'ascii') for name in names] for names in [d[1] for d in spec]]
  iat_tables = [(u64*(len(d[1])+1))(*([0]*(len(d[1])+1))) for d in spec]
  module_names = [bytes(mname + '\0', 'ascii') for mname, fnames in spec]

  # calculate total size for idata segment
  idata_size = descriptors.size() + \
    sum(v.size() for v in int_tables) + \
    sum(len(v) for v in flatten(image_import_by_name_array)) + \
    sum(v.size() for v in iat_tables) + \
    sum(len(v) for v in module_names)
  return idata_size, descriptors, int_tables, image_import_by_name_array, iat_tables, module_names


# Assembly for the .text you will see below
# NOTE: all addresses will get patched later 
# replacing these stub addresses with correct ones
# 
# sub rsp, 0x28 ; stack shadowspace / alignment 
# mov r9d, 0
# mov r8d, 0x000000 ; Pointer to "a simple 64b PE executable"
# mov edx, 0x000000 ; Pointer to "Hello World!"
# mov ecx, 0
# call [0x000000] ; MessageBoxA(0, "a simple PE executable", "Hello World!", 0)
# mov ecx, 0
# call [0x000000] ; ExitProcess(0)

# NOTE: Although you can input multiple functions, there is currently a but within the 
# int_table where only one of the addresses is calculated (the first one) 
imports_spec = [
  ['kernel32.dll', ['ExitProcess']],
  ['user32.dll',   ['MessageBoxA']],
]

# NOTE: Here the function addresses are NULL, the final code will have the real function addresses patched later
_text = ''              +\
  '4883EC' + '28'       +\
  '41B9'   + '00000000' +\
  '41B8'   + '00000000' +\
  'BA'     + '00000000' +\
  'B9'     + '00000000' +\
  'FF1425' + '00000000' +\
  'B9'     + '00000000' +\
  'FF1425' + '00000000'

hello_world              = b'Hello World!\0'
simple_64b_pe_executable = b'a simple 64b PE executable\0'
_data = simple_64b_pe_executable + hello_world 

pe_file_header     = IMAGE_FILE_HEADER()
pe_optional_header = IMAGE_OPTIONAL_HEADER64()

pe_file_header.NumberOfSections = 3 # .text, .data, .idata sections 
pe_file_header.SizeOfOptionalHeader = pe_optional_header.size()

pe_optional_header.AddressOfEntryPoint = 0x1100 # look at why this address works but not 0x1000 or a higher address
pe_optional_header.BaseOfCode    = 0x1000 # default
pe_optional_header.SizeOfImage   = 0x4000 # this should be calculated later...
pe_optional_header.SizeOfHeaders = 0x300  # this should be calculated later...


# Sections
byte_array_x8_type = u8*8
byte_array_x8 = lambda x: byte_array_x8_type(*([u8(c) for c in x] + [0]*(8-len(x))*(len(x) < 8)))
file_alignment = pe_optional_header.FileAlignment.value
section_alignment = pe_optional_header.SectionAlignment.value

# .text
pe_section_table_text = IMAGE_SECTION_HEADER()
pe_section_table_text.Name = byte_array_x8('.text')
pe_section_table_text.VirtualSize    = len(_text)//2
pe_section_table_text.VirtualAddress = 0x1000
pe_section_table_text.SizeOfRawData  = align(len(_text)//2, 0x100)
pe_section_table_text.PointerToRawData = pe_optional_header.SizeOfHeaders.value # 0x300
pe_section_table_text.Characteristics = Characteristics_Flags.IMAGE_SCN_MEM_EXECUTE | \
                                        Characteristics_Flags.IMAGE_SCN_MEM_READ | \
                                        Characteristics_Flags.IMAGE_SCN_CNT_CODE

idata_size, descriptors, int_tables, image_import_by_name_array, iat_tables, module_names = gen_empty_import_structure(imports_spec)

# .idata
import_structure_base_address = align(pe_section_table_text.VirtualAddress.value + pe_section_table_text.SizeOfRawData.value, section_alignment) # 0x2000
pe_section_table_idata = IMAGE_SECTION_HEADER()
pe_section_table_idata.Name = byte_array_x8('.idata')
pe_section_table_idata.VirtualSize    = idata_size
pe_section_table_idata.VirtualAddress = import_structure_base_address
pe_section_table_idata.SizeOfRawData  = align(idata_size, file_alignment)
pe_section_table_idata.PointerToRawData = pe_section_table_text.PointerToRawData.value + pe_section_table_text.SizeOfRawData.value
pe_section_table_idata.Characteristics = Characteristics_Flags.IMAGE_SCN_MEM_READ | \
                                        Characteristics_Flags.IMAGE_SCN_CNT_INITIALIZED_DATA

# make sure that the idata address is the same for the import's table
dd = pe_optional_header.DataDirectory
dd[1].VirtualAddress = import_structure_base_address # importsVA
dd[1].Size = idata_size # TODO: Figure out whether this matters or not
pe_optional_header.DataDirectory = dd

# .data
pe_section_table_data = IMAGE_SECTION_HEADER()
pe_section_table_data.Name = byte_array_x8('.data')
pe_section_table_data.VirtualSize    = len(_data)//2
pe_section_table_data.VirtualAddress = align(pe_section_table_idata.VirtualAddress.value + pe_section_table_idata.SizeOfRawData.value, section_alignment) # 0x3000
pe_section_table_data.SizeOfRawData  = align(len(_data)//2, file_alignment)
pe_section_table_data.PointerToRawData = pe_section_table_idata.PointerToRawData.value + pe_section_table_idata.SizeOfRawData.value
pe_section_table_data.Characteristics = Characteristics_Flags.IMAGE_SCN_MEM_WRITE | \
                                        Characteristics_Flags.IMAGE_SCN_MEM_READ  | \
                                        Characteristics_Flags.IMAGE_SCN_CNT_INITIALIZED_DATA

# Fill import structure

descriptors_address = import_structure_base_address

def calc_addresses(base_address, obj_list: list):
  obj_addresses = []
  for v in obj_list: 
    obj_addresses += [base_address]
    base_address += len(v) if type(v) == str or type(v) == bytes else v.size() 
  return obj_addresses, base_address

int_tables_base_address = descriptors_address + descriptors.size()
int_tables_addresses,   last_address = calc_addresses(int_tables_base_address, int_tables)
function_name_addresses,last_address = calc_addresses(last_address, flatten(image_import_by_name_array))
iat_tables_addresses,   last_address = calc_addresses(last_address, iat_tables)
module_names_addresses, last_address = calc_addresses(last_address, module_names)

# fill out descriptors
for i in range(len(descriptors)-1): 
  descriptors[i].OriginalFirstThunk = int_tables_addresses[i]
  descriptors[i].Name               = module_names_addresses[i]
  descriptors[i].FirstThunk         = iat_tables_addresses[i]

# fill out int_tables
index = 0
for i, table in enumerate(int_tables):
  for j in range(len(table)-1):
    table[j] = function_name_addresses[index]
    index += 1

# fill out iat_tables
index = 0
for i, table in enumerate(iat_tables):
  for j in range(len(table)-1):
    table[j] = function_name_addresses[index]
    index += 1
  
def reduce(l):
  acc = bytearray()
  for val in l: acc += val
  return acc

_idata = descriptors.get_bytes() + \
  reduce([v.get_bytes() for v in int_tables]) + \
  reduce([v for v in flatten(image_import_by_name_array)]) + \
  reduce([v.get_bytes() for v in iat_tables]) + \
  reduce([v for v in module_names])

# calculate final addressses for functions 
all_function_addresses = iat_tables_addresses
address = [hex(int.from_bytes(int.to_bytes(v+pe_optional_header.ImageBase.value, byteorder='little', length=4), byteorder='big'))[2:].rjust(4*2, '0') for v in all_function_addresses]

print([hex(v) for v in int_tables_addresses])
print([hex(v) for v in function_name_addresses])
print([hex(v) for v in iat_tables_addresses])
print([hex(v) for v in module_names_addresses])

exit_process, message_box_a = address # final functions addresses
# calculate final addresses for values in the data segment
data_seg_base = pe_optional_header.ImageBase.value + pe_section_table_data.VirtualAddress.value
def data_seg_address(x): 
  global data_seg_base
  result = data_seg_base
  data_seg_base += len(x)
  return hex(int.from_bytes(int.to_bytes(result, length=4, byteorder='little'), byteorder='big'))[2:].rjust(4*2, '0')
  
simple_64b_pe_executable_address = data_seg_address(simple_64b_pe_executable)
hello_world_address = data_seg_address(hello_world)

_text = bytes.fromhex(''+\
  '4883EC' + '28'       +\
  '41B9'   + '00000000' +\
  '41B8'   +  hello_world_address              +\
  'BA'     +  simple_64b_pe_executable_address +\
  'B9'     + '00000000' +\
  'FF1425' +  message_box_a +\
  'B9'     + '00000000' +\
  'FF1425' +  exit_process)

print(address)
print(simple_64b_pe_executable_address)
print(hello_world_address)



image_dos_signiture = IMAGE_DOS_SIGNITURE
image_dos_stub = IMAGE_DOS_SUB
pe_signiture = PE_SIGNITURE
pe_file_header = pe_file_header.get_bytes()
pe_optional_header = pe_optional_header.get_bytes()
pe_section_table = pe_section_table_text.get_bytes() + \
          pe_section_table_idata.get_bytes() +\
          pe_section_table_data.get_bytes() 

def pad(x: bytearray, length: int, padding=b'\0'): # add padding 
  return x + (padding * (length - len(x))) if (length - len(x)) > 0 else x 

result = image_dos_signiture + \
        image_dos_stub       + \
        pe_signiture         + \
        pe_file_header       + \
        pe_optional_header   + \
        pe_section_table
result  = pad(result, pe_section_table_text.PointerToRawData.value)
result += pad(_text, pe_section_table_text.SizeOfRawData.value, b'\xcc') + \
          pad(_idata, pe_section_table_idata.SizeOfRawData.value)        + \
          pad(_data, pe_section_table_data.SizeOfRawData.value)

file_path = 'test.exe'
with open(file_path,"wb") as f:
  f.write(result)
