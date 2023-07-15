from enum import IntEnum, Enum

INT32_MAX = 2**32

Register = IntEnum(
  "Register", [
  # REX.r = 0 or REX.b = 0 or REX.x = 0
  'AL',  'CL',  'DL',  'BL',  'AH',  'CH',  'DH',  'BH',
  '_',   '__',  '___', '____', 'SPL', 'BPL', 'SIL', 'DIL',
  'AX',  'CX',  'DX',  'BX',  'SP',  'BP',  'SI',  'DI',
  'EAX', 'ECX', 'EDX', 'EBX', 'ESP', 'EBP', 'ESI', 'EDI',
  'RAX', 'RCX', 'RDX', 'RBX', 'RSP', 'RBP', 'RSI', 'RDI',
  # REX.r = 1 or REX.b = 1 or REX.x = 1
  'R8B', 'R9B', 'R10B', 'R11B', 'R12B', 'R13B', 'R14B', 'R15B',
  'R8W', 'R9W', 'R10W', 'R11W', 'R12W', 'R13W', 'R14W', 'R15W',
  'R8D', 'R9D', 'R10D', 'R11D', 'R12D', 'R13D', 'R14D', 'R15D',
  'R8',  'R9',  'R10',  'R11',  'R12',  'R13',  'R14',  'R15'
])

def get_reg(x): # returns reg number (1-7), bitness (8, 16, 32, 64), whether the register is a r8-r15 
  reg, bitness = (x.value-1) % 8, (x.value-1) // 8
  return reg, 2**[0, 0, 1, 2, 3, 0, 1, 2, 3, 3][bitness]*8, (bitness > 4) & 1

class Address: 
  def __init__(self, base=None, disp=0, index=None, scale=None):
    assert (isinstance(base, Register) or base is None) and (isinstance(index, Register) or index is None)
    assert type(disp) == int and disp < INT32_MAX
    assert scale is None or scale < 4, f'Scale must at most 3 but got {scale}'

    if base is not None and index is not None: 
      _, bitness1, _ = get_reg(base)
      _, bitness2, _ = get_reg(index)

      assert bitness1 == bitness2, f'Registers ({base.name}, {index.name}) must be of the same size ({bitness1} != {bitness2})'

    self.disp, self.index, self.base = disp, index, base 
    self.scale = scale if scale is not None else 0 
  def __repr__(self):
    get_string = lambda x: x.name if isinstance(x, IntEnum) else x if x is not None else ''
    disp, index, scale, base = get_string(self.disp), get_string(self.index), get_string(self.scale), get_string(self.base)
    return f'({base}{"+" if base is not "" else ""}{index}{"*" if scale is not "" else ""}{scale*2}{"+" if disp != 0 else ""}{hex(disp)})'


def to_byte(x): return int.to_bytes(x, length=1, byteorder='little')
def to_bytes(x, l=0): return int.to_bytes(x, length=l, byteorder='little')

OPCODE_W = 1 << 0
OPCODE_S = 1 << 1
OPCODE_D = 1 << 2

# 
# Mod R/m Byte
#
def get_mod_rm(operand1, operand2=None, imm_reg_opcode=0):


  # register to register
  if isinstance(operand1, Register) and isinstance(operand2, Register):
    r_m = get_reg(operand1)[0]
    reg = get_reg(operand2)[0]
    mod = 0b11 # ; r->r
  
  # immediate to register e.g. add al, 0FFh
  elif (isinstance(operand1, Register) and (isinstance(operand2, int) or isinstance(operand2, bytes))) or \
     (isinstance(operand1, Register) and operand2 == None):
    mod, reg = 0b11, imm_reg_opcode
    r_m = get_reg(operand1)[0]

  else:

    # immediate to memory  
    if isinstance(operand1, Address) and isinstance(operand2, bytes): 
      address = operand1
      reg = imm_reg_opcode

    # memory to register or register to memory e.g. mov eax, [rbp+0x100]
    elif (isinstance(operand1, Register) and isinstance(operand2, Address)) or (isinstance(operand2, Register) and isinstance(operand1, Address)):
      register, address = operand1, operand2 
      if isinstance(operand2, Register) and isinstance(operand1, Address):
        register, address = address, register
      reg = get_reg(register)[0]

    elif isinstance(operand1, Address): # unary operation
      address = operand1
      reg = imm_reg_opcode

    base, index, disp = address.base, address.index, address.disp
    r_m = get_reg(base)[0] if base else 0
    
    if base is None and index is None: mod= 0b00
    elif disp >= 256:                  mod= 0b10 # 10 - [eax] + dips32 ; r->m/m->r
    elif 0 < disp and disp < 256:      mod= 0b01 # 01 - [eax] + dips8  ; r->m/m->r
    elif disp == 0:                    mod= 0b00 # 00 - [eax]          ; r->m/m->r

    if index is not None or base == Register.RSP or (base is None and index is None): # use sib byte
      r_m = 0b100 # r_m == 0b100 because this will use the sib byte

  ModRM = to_byte(mod << 6 | reg << 3 | r_m << 0)
  return ModRM

#
# SIB Byte
# 
def get_sib(address):
  base, index, scale = address.base, address.index, address.scale

  # if you don't need to use the sib byte, it will not return a sib byte
  if not (index is not None or base == Register.RSP or (base is None and index is None)):
    return b''
    
  # remove base using 0b101 value 
  base = get_reg(base)[0] if base else 0b101
  # remove index using 0b100 value
  index = get_reg(index)[0] if index else 0b100

  sib = to_byte(scale << 6 | index << 3 | base << 0)
  return sib


# To remember from the intel manual, how rex effects ModRM and SIB bytes
# 1. register to register addressing
# rex       | opcode     | mod reg r/m
# 0b100wr0b | 0b00000dsw | 11  rrr bbb
#
# 2. memory addressing wihout a sib byte
# rex       | opcode     | mod  reg r/m
# 0b100wr0b | 0b00000dsw | !=11 rrr bbb
#
# 3. memory addressing with a sib byte
# rex       | opcode     | mod  reg r/m | scale index base
# 0b100wrxb | 0b00000dsw | !=11 rrr 100 | ss    xxx   bbb


# address_size - only when writing unary instruction with a memory

# limit_width=True limits the size of a constant for the instrtuctions to 32bit (as it is generally wanted)
# opcode_touch=True changes the OPCODE_W bit of the opcode depending on which size is needed
def write_instruction(opcode, operand1, operand2=None, mod_rm=b'', sib=b'', address_size=0, limit_width=True, opcode_touch=True):
  rex_b, rex_r, rex_x = 0, 0, 0 # default to 0
  address_bitness, disp = 0, 0
  length = address_size
  constant = b''

  # register to register
  if isinstance(operand1, Register) and isinstance(operand2, Register):
    reg_to,   bitness_to,   rex_b = get_reg(operand1)
    reg_from, bitness_from, rex_r = get_reg(operand2)
    assert bitness_to == bitness_from, \
    f'Registers {operand1.name}, {operand2.name} are not of the same size ({bitness_to} != {bitness_from})'

  # immediate to register e.g. add al, 0FFh
  elif isinstance(operand1, Register) and (isinstance(operand2, int) or isinstance(operand2, bytes)) :
    r_m, bitness_to, rex_b = get_reg(operand1)

    if isinstance(operand2, int):
      constant = to_bytes(operand2, l=min(bitness_to//8, 4 if limit_width else 8))
    else: 
      constant = operand2
  
  elif isinstance(operand1, Register) and operand2 == None:
    register_to, bitness_to, rex_b = get_reg(operand1)

  # memory operations
  else: 
    # immediate to memory 
    if isinstance(operand1, Address) and isinstance(operand2, bytes): 
      address, constant = operand1, operand2 
      bitness_to = len(constant) * 8
    # Address to Register or Register to Address e.g. mov eax, [rbp+0x100]
    elif (isinstance(operand1, Register) and isinstance(operand2, Address)) or (isinstance(operand2, Register) and isinstance(operand1, Address)):
      register, address = operand1, operand2 
      reg, bitness_to, rex_r = get_reg(register)
    # unary memorry
    elif isinstance(operand1, Address) and operand2 == None:
      bitness_to = address_size*8
      address = operand1
    else: 
      raise Exception("Bug?")

    base, index, disp = address.base, address.index, address.disp
    # we need to handle the address part that both operations have here
    address_bitness, rex_b = get_reg(base)[1:] if base else (0,0)

    length = address_size
    if base is None and index is None: length=4
    elif disp >= 256:                  length=4 # [eax] + dips32 ; r->m/m->r
    elif 0 < disp and disp < 256:      length=1 # [eax] + dips8  ; r->m/m->r
    elif disp == 0 and address_size==0:length=0 # [eax]          ; r->m/m->r

    if index is not None or base == Register.RSP or (base is None and index is None): # use sib byte
      if index is None: index, rex_x = 0b100, 0 # remove index using 0b100 value
      else: index, _, rex_x = get_reg(index)

  is_inst_64bit = bitness_to == 64
  prefix_bin  = to_byte(0x66) if bitness_to == 16 else b'' + to_byte(0x67) if address_bitness == 32 else b''
  rex_bin     = to_byte((1 << 6) | # Must have for it to be recognized as rex byte
                        (1 << 0)*rex_b | 
                        (1 << 1)*rex_x | 
                        (1 << 2)*rex_r | 
                        (1 << 3)*is_inst_64bit
                        ) if rex_b or rex_r or rex_x or is_inst_64bit else b''
  opcode_bin  = to_byte(opcode) if bitness_to == 8 else to_byte(opcode | OPCODE_W) if opcode_touch else to_byte(opcode)
  operand_bin = mod_rm
  sib_bin     = sib
  constant_bin = constant
  displace_bin = to_bytes(disp, l=length) if disp or address_size else b''


  return prefix_bin + rex_bin + opcode_bin + operand_bin + sib_bin + displace_bin + constant_bin 

# from collections import defaultdict
class Builder: 
  def __init__(self, imports=None): 
    self.import_function_addresses_to_patch = []
    self.buff = bytearray()

    self.function_addresses = {}
    for lib, funcs in imports: 
      for func in funcs: 
       self.function_addresses[func] = 0

    self.data_addresses_to_patch = []
    self.data_args = {}

  def add_data(self, var_name, data):
    self.data_args[var_name] = bytes((data + '\0').encode('ascii'))

  def __call__(self, import_function_addresses, data_seg_base, text_seg_base):
    self.function_addresses = import_function_addresses

    # patch locations that need to be patched
    for name, patch in self.import_function_addresses_to_patch:
      self.buff[slice(*patch)] = to_bytes(self.function_addresses[name], l=patch[1]-patch[0]) 


    for data in self.data_args.keys():
      print(data)

    for name, patch in self.data_addresses_to_patch:
      self.buff[slice(*patch)] = to_bytes(self.data_args[name], l=patch[1]-patch[0]) 
    return self.buff

  # 
  # Binary 
  # 

  def general_instruction_binary(self, opcode_t, operand1, operand2): 
    opcode, imm_opcode, imm_reg = opcode_t

    assert isinstance(operand1, Register) or isinstance(operand1, Address), \
      f'First operand {operand1}, must be a Register or Address'
    
    # register to register
    if isinstance(operand1, Register) and isinstance(operand2, Register):
      output = write_instruction(opcode, operand1, operand2, mod_rm=get_mod_rm(operand1, operand2))

    # immediate to register e.g. add al, 0FFh
    elif isinstance(operand1, Register) and isinstance(operand2, int):
      register, constant = operand1, operand2

      reg, bitness, rex_b = get_reg(register)
      assert constant < 2**bitness, f'Constant 0x{constant:x} too big for {bitness}bit register {register.name}'
        
      if reg == 0 and rex_b == 0: # special case for {al, ax, eax, rax} regs
        output = write_instruction(opcode | OPCODE_D, register, constant)
      else: # regular cases

        # if found to be needed, operate on sign extend imm8 to r16/r32/r64
        if constant < 127 and bitness > 8 and (opcode == 0x28 or opcode==0x00): 
          imm_opcode |= OPCODE_S
          constant = to_byte(constant)

        output = write_instruction(imm_opcode, register, constant, mod_rm=get_mod_rm(register, constant, imm_reg_opcode=imm_reg))
     
    # immediate to memory e.g. add [rax], 0FFh
    elif isinstance(operand1, Address) and ((isinstance(operand2, int) or isinstance(operand2, bytes)) and not isinstance(operand2, Register)):
      address, constant = operand1, operand2

      assert isinstance(constant, bytes), f'Got int {constant} with unknown size, please give the number in bytes type'
      # TODO: I might want to use the imm8 sign extend to r16/r32/r64 whilch will be opcode (0x80 | OPCODE_W | OPCODE_S) but until then... 
      output = write_instruction(imm_opcode, address, constant,
                                  mod_rm=get_mod_rm(address, constant, imm_reg_opcode=imm_reg), 
                                  sib=get_sib(address))

    # Address to Register or Register to Address e.g. mov eax, [rbp+0x100]
    elif (isinstance(operand1, Register) or isinstance(operand1, Address)) and (isinstance(operand2, Register) or isinstance(operand2, Address)):
      register, address = operand1, operand2 

      if isinstance(register, Register) and isinstance(address, Address): opcode |= OPCODE_S # switch positions 
      else: register, address = address, register

      output = write_instruction(opcode, register, address, 
                                mod_rm=get_mod_rm(operand1, operand2), 
                                sib=get_sib(address))
    else: 
      raise Exception("Bug? or not implemented")

    self.buff += output 
    return output

  def MOV(self, operand1, operand2):

    fname = None
    dname = None
    address_size = 0
    if type(operand2) == str:
      if operand2 in self.function_addresses.keys():
        fname = operand2
        operand2 = Address(disp=self.function_addresses[operand2])
        address_size = 4
      elif operand2 in self.data_args.keys(): 
        dname = None
        operand2 = Address(disp=0)
        address_size = 4


    if isinstance(operand1, Register) and (isinstance(operand2, int) and not isinstance(operand2, Register)):
      register, constant = operand1, operand2

      reg, bitness, _ = get_reg(register)
      opcode = 0xb8 + reg
      if constant < 256 and bitness == 8: 
        opcode = 0xb0 + reg

      result = write_instruction(opcode, operand1, operand2, opcode_touch=False, limit_width=False, address_size=address_size)
    else: 
      result = self.general_instruction_binary((0x88, 0xc6, 0x0), operand1, operand2)

    if fname is not None:  
      self.import_function_addresses_to_patch += [(fname, (len(self.buff) - address_size, len(self.buff)))]
    elif dname is not None: 
      self.data_addresses_to_patch += [(dname, (len(self.buff) - address_size, len(self.buff)))]
    
    self.buff += result
    return result

  # 
  # Unary
  #

  def general_instruction_unary(self, opcode, operand, address_size=0):
    opcode, opcode_reg = opcode

    sib = b''
    if isinstance(operand, Address):
      assert address_size > 0, f'Must specify the size '
      sib = get_sib(operand)

    return write_instruction(opcode, operand, 
                                  mod_rm=get_mod_rm(operand, imm_reg_opcode=opcode_reg), 
                                  sib=sib, address_size=address_size)

  def NOT(self, operand, address_size=0):
    opcode = 0xf6, 0x2
    output = self.general_instruction_unary(opcode, operand, address_size)
    self.buff += output 
    return output
  
  def CALL(self, operand, address_size=0):
    # TODO: This does work on the common case but basically fails miserably on almost anything else 
    # so meh fix it I guess 
    opcode = 0xff, 0x2

    patch_address = False
    if type(operand) == str and self.function_addresses.get(operand) is not None: # function name
      function_name = operand
      operand = Address(disp=self.function_addresses[operand])
      address_size = 4
      patch_address = True
    elif type(operand) == str: 
      raise Exception(f"Invalid function name given, '{operand}' does not exist")

    output = self.general_instruction_unary(opcode, operand, address_size)
    self.buff += output 

    if patch_address: 
      self.import_function_addresses_to_patch += [(function_name, (len(self.buff) - address_size, len(self.buff)))]
    return output
 
gp_inst_t = {
  'ADD' : (0x00, 0x80, 0x0),
  'OR'  : (0x08, 0x80, 0x1),
  'AND' : (0x20, 0x80, 0x4),
  'SUB' : (0x28, 0x80, 0x5),
  'XOR' : (0x30, 0x80, 0x6),
  'CMP' : (0x38, 0x80, 0x7)
}

def register_gi(k, v): setattr(Builder, k, lambda self, oper1, oper2: self.general_instruction_binary(v, oper1, oper2))
for k, v in gp_inst_t.items(): register_gi(k, v)


# TODO: add these: 
# call TODO: fix the call 
# mul
# div
# test 
# jmp
# movsx 
# movzx 
# push 
# pop
# ret 
# neg
# je    <- maybe just all the variations or something like that


imports = [
  ['kernel32.dll', ['ExitProcess']],
  ['user32.dll',   ['MessageBoxA']],
]

builder = Builder(imports)

# builder.add_data('hello_world',   'Hello World!')
# builder.add_data('simple_pe_exe', 'A simple 64b pe exe')

builder.SUB(Register.RSP, 0x28)
builder.MOV(Register.R9D, 0)
builder.MOV(Register.R8D, 'hello_world')
builder.MOV(Register.EDX, 'simple_pe_exe')
builder.MOV(Register.ECX, 0)
builder.CALL('MessageBoxA')
builder.MOV(Register.ECX, 0)
builder.CALL('ExitProcess')

import_function_addresses = { 'ExitProcess' : 0x3000, 'MessageBoxA' : 0x3010 } 


# INST = builder.XOR

# INST(Register.R9, 0x700)
# INST(Register.CL, Register.AL)
# INST(Register.DL, Register.R9B)
# INST(Register.EAX, Register.R9D)
# INST(Register.RDI, Register.R15)
# INST(Register.CL , 0x70)
# INST(Register.EAX, 0x70)
# INST(Register.R9, 0x700)
# INST(Register.EAX, Address(Register.RAX))
# INST(Register.EAX, Address(disp=0x400))
# INST(Register.RAX, Address(Register.RCX, disp=0x100))
# INST(Register.RAX, Address(Register.RCX, index=Register.RAX, scale=3, disp=0x100))
# INST(Register.EAX, Address(Register.R9, index=Register.RBP, scale=1, disp=0x400))
# INST(Register.EAX, Address(Register.RSP))
# INST(Register.R10, Address(Register.RSP, disp=100))
# INST(Register.RAX, Address(Register.RSP, index=Register.RBP))
# INST(Register.EAX, Address(Register.RSP, index=Register.R15, disp=100))
# INST(Address(Register.RSP, index=Register.R15, disp=100), Register.EAX)
# INST(Address(Register.RAX, disp=0x100, index=Register.RAX, scale=1), to_bytes(0x70, l=1))
# INST(Address(Register.R15, disp=0x100, index=Register.R9), to_bytes(0x70, l=4))

builder.ADD(Register.EAX, Register.ECX)
builder.SUB(Register.EAX, Register.ECX)
builder.OR(Register.EAX, Register.ECX)
builder.CMP(Register.EAX, Register.ECX)
builder.AND(Register.EAX, Register.ECX)
builder.MOV(Register.EAX, 0x100)
builder.NOT(Address(Register.RAX, index=Register.RCX, disp=0x10), address_size=1)
builder.XOR(Register.EAX, 0x100)
builder.XOR(Register.EAX, Register.EAX)
builder.NOT(Register.EAX)

# using iced_x86 decoder to make sure the machine code I output
# is correct (for development only)
def decode(inst):
  EXAMPLE_CODE = inst

  from iced_x86 import Decoder, Formatter,  FormatterSyntax

  EXAMPLE_CODE_BITNESS = 64
  EXAMPLE_CODE_RIP = 0x0 #0x0000_7FFA_C46A_CDA4
  decoder = Decoder(EXAMPLE_CODE_BITNESS, EXAMPLE_CODE, ip=EXAMPLE_CODE_RIP)

  formatter = Formatter(FormatterSyntax.MASM)

  formatter.digit_separator = "`"
  formatter.first_operand_char_index = 10
  print()
  print(f'EXAMPLE_CODE={EXAMPLE_CODE}')
  print()
  for instr in decoder:
      disasm = formatter.format(instr)
      start_index = instr.ip - EXAMPLE_CODE_RIP
      bytes_str = EXAMPLE_CODE[start_index:start_index + instr.len].hex().upper()
      print(f"{instr.ip:016X} {bytes_str:20} {disasm}")
  # decoder = Decoder(64, b"\x86\x64\x32\x16", ip=0x1234_5678)
  # instr = decoder.decode()
  # print(instr)

decode(builder(import_function_addresses, data_seg_base=0x3000, text_seg_base=0x1000))