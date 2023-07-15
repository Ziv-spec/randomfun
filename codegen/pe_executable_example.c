// Simple PE executable writer (example of https://github.com/corkami/pics/blob/master/binary/pe101/pe101-64.png)
#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <winnt.h>

typedef struct { 
	char *dll_name;
	char **function_names;
} Entry; 


#if DEBUG
#include <stdio.h>
static char error_buffer[0x100];
#define Assert(condition, msg, ...) \
do {                        \
if (!(condition)) {      \
sprintf(error_buffer, msg, __VA_ARGS__); \
MessageBoxA(0, error_buffer, 0, MB_ICONERROR ); \
__debugbreak(); \
} \
} while(0)
#else 
#define Assert(condition, msg, ...)
#endif 

#define MOVE(dest, src, size) do { memcpy(dest, src, size); dest += size; } while(0)

static inline int align(int x, int alignment) {
	int remainder = x % alignment; 
	return remainder ? (x - remainder + alignment) : x;
}

int main(int argc, char *argv[]) {
	
	//~
	// User defined data to put into executable
	// 
	
	char *file = "test.exe"; // default
	if (argc > 1) {
		file = argv[1];
	}
	
	// data section info
	char hello_world[]   = "Hello World!";
	char simple_pe_exe[] = "a simple 64b PE executable";
	char data[sizeof(hello_world) + sizeof(simple_pe_exe)], *pdata = data;
	MOVE(pdata, simple_pe_exe, sizeof(simple_pe_exe)); 
	MOVE(pdata, hello_world,   sizeof(hello_world)); 
	
	// text section info
	char code[] = {
		0x48, 0x83, 0xec, 0x28,                // sub rsp, 0x28
		0x41, 0xb9, 0, 0, 0, 0,                // mov r9d, 0
		0x41, 0xb8, 0, 0x30, 0x40, 0,          // mov r8d, 0x403000  ; "a simple 64b PE executable"
		0xba, 0x1b, 0x30, 0x40, 0,             // mov edx, 0x43001b  ; "Hello World!"
		0xb9, 0, 0, 0, 0,                      // mov ecx, 0
		0xff, 0x14, 0x25, 0x88, 0x20, 0x40, 0, // call 0x402088      ; MessageBoxA
		0xb9, 0, 0, 0, 0,                      // mov ecx, 0
		0xff, 0x14, 0x25, 0x78, 0x20, 0x40, 0, // call 0x402078      ; ExitProcess
	};
	
	// idata section info, this defines functions you want to import & their respective dll's
	Entry entries[] = {
		{"kernel32.dll", (char *[]){ "ExitProcess", 0 }},
		{"user32.dll",   (char *[]){ "MessageBoxA", 0 }}, 
	};
	
	// sizes
	int code_size = sizeof(code), data_size = sizeof(data);
	int entries_count = sizeof(entries)/sizeof(entries[0]); 
	
	
	//~
	// PE executable NT signiture, DOS stub, NT headers
	//
	
	// NOTE(ziv): the first two bytes of NT signiture stand for 
	// MZ which is used to identify the file as a executable
	static char image_stub_and_signiture[] = {
		// image NT signiture
		0x4D, 0x5A, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 
		0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// image dos stub
		0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68,
		0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F, 
		0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20, 
		0x6D, 0x6F, 0x64, 0x65, 0x2E, 0x0D, 0x0D, 0x0A, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x86, 0x07, 0x08, 0xF2, 0xC2, 0x66, 0x66, 0xA1, 0xC2, 0x66, 0x66, 0xA1, 0xC2, 0x66, 0x66, 0xA1, 
		0xD6, 0x0D, 0x62, 0xA0, 0xC9, 0x66, 0x66, 0xA1, 0xD6, 0x0D, 0x65, 0xA0, 0xC5, 0x66, 0x66, 0xA1, 
		0xD6, 0x0D, 0x63, 0xA0, 0x52, 0x66, 0x66, 0xA1, 0xA0, 0x1E, 0x63, 0xA0, 0xE5, 0x66, 0x66, 0xA1, 
		0xA0, 0x1E, 0x62, 0xA0, 0xD2, 0x66, 0x66, 0xA1, 0xA0, 0x1E, 0x65, 0xA0, 0xCB, 0x66, 0x66, 0xA1, 
		0xD6, 0x0D, 0x67, 0xA0, 0xCB, 0x66, 0x66, 0xA1, 0xC2, 0x66, 0x67, 0xA1, 0xB1, 0x66, 0x66, 0xA1, 
		0xA3, 0x1C, 0x63, 0xA0, 0xC3, 0x66, 0x66, 0xA1, 0xA3, 0x1C, 0x64, 0xA0, 0xC3, 0x66, 0x66, 0xA1, 
		0x52, 0x69, 0x63, 0x68, 0xC2, 0x66, 0x66, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	};
	// Set the address at location 0x3c to be after signiture and stub
	*(unsigned int *)&image_stub_and_signiture[0x3c] = sizeof(image_stub_and_signiture);
	
	IMAGE_NT_HEADERS nt_headers = {
		.Signature = 'P' | 'E' << 8,
		
		.FileHeader = { 
			.Machine = IMAGE_FILE_MACHINE_AMD64,
			.NumberOfSections     = 3, // in our case we need a .text .data .idata
			.SizeOfOptionalHeader = 0,
			.Characteristics      = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE
		}, 
		
		.OptionalHeader = {
			.Magic = IMAGE_NT_OPTIONAL_HDR_MAGIC, 
			
			// NOTE(ziv): These are not important
			.MajorOperatingSystemVersion = 6,
			.MajorSubsystemVersion = 6,
			
			.ImageBase        = 0x400000,
			.SectionAlignment = 0x1000,   // Minimum space that can a section can occupy when loaded that is, 
			.FileAlignment    = 0x200,    // .exe alignment on disk
			.SizeOfImage      = 0,
			.SizeOfHeaders    = 0,
			
			.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI, 
			.SizeOfStackReserve = 0x100000,
			.SizeOfStackCommit  = 0x1000,
			.SizeOfHeapReserve  = 0x100000,
			.SizeOfHeapCommit   = 0x1000,
			
			.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES,
			.DataDirectory = {0}
		}
	};
	
	int section_alignment = nt_headers.OptionalHeader.SectionAlignment; 
	int file_alignment    = nt_headers.OptionalHeader.FileAlignment; 
	
	// special numbers that are not known ahead of time and will have to get calculated
	int sizeof_headers_unaligned = sizeof(image_stub_and_signiture) + sizeof(nt_headers) + 
		nt_headers.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER);
	{
		nt_headers.FileHeader.SizeOfOptionalHeader = sizeof(nt_headers.OptionalHeader);
		
		nt_headers.OptionalHeader.AddressOfEntryPoint = 0x1000;
		nt_headers.OptionalHeader.SizeOfImage   = 0x4000; // TODO(ziv): Make automatic
		nt_headers.OptionalHeader.SizeOfHeaders = align(sizeof_headers_unaligned, file_alignment);
	}
	
	
	//~
	// .text .data .idata section headers
	// 
	
	// NOTE(ziv): idata section structure
	// Import Directory Table - contains information about the following structures locations
	// Import Name Table - table of references to tell the loader which functions are needed (NULL terminated)
	// Function Names - A array of the function names we want the loader to load. The structure is 'IMAGE_IMPORT_BY_NAME' (it's two first bytes are a 'HINT')
	// Import Address Table - identical to Import Name Table but during loading gets overriten with valid values for function addresses
	// Module names - simple array of the module names
	
	// Compute sizes for all idata related structures
	int descriptors_size = sizeof(IMAGE_IMPORT_DESCRIPTOR) * (entries_count + 1);
	int int_tables_size = 0, module_names_size = 0, function_names_size = 0; 
	for (int i = 0; i < entries_count; i++) {
		char **fnames = entries[i].function_names, **fnames_temp = fnames;
		for (; *fnames_temp; fnames_temp++) 
			function_names_size += (int)(strlen(*fnames_temp) + 1 + 2); // +2 for 'HINT'
		int_tables_size += (int)(sizeof(size_t) * (fnames_temp-fnames+1));
		module_names_size += (int)(strlen(entries[i].dll_name) + 1);
	}
	int idata_size = descriptors_size + int_tables_size*2 + function_names_size + module_names_size;
	
	
	IMAGE_SECTION_HEADER text_section = { 
		.Name = ".text",
		.Misc.VirtualSize = 0x1000, // set actual size 
		.VirtualAddress   = section_alignment, 
		.SizeOfRawData    = align(code_size, file_alignment),
		.PointerToRawData = nt_headers.OptionalHeader.SizeOfHeaders, // the sections begin right after all the NT headers
		.Characteristics  = IMAGE_SCN_MEM_EXECUTE |IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_CODE
	}; 
	
	int idata_section_address = align(text_section.VirtualAddress + text_section.SizeOfRawData, section_alignment);
	IMAGE_SECTION_HEADER idata_section = { 
		.Name = ".idata",
		.Misc.VirtualSize = idata_size, 
		.VirtualAddress   = idata_section_address,
		.SizeOfRawData    = align(idata_size, file_alignment),
		.PointerToRawData = text_section.PointerToRawData + text_section.SizeOfRawData,
		.Characteristics  = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA,
	}; 
	
	// importsVA - make sure that idata section is recognized as a imports section
	{
		nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = idata_section_address; 
		nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = idata_size; 
	}
	
	IMAGE_SECTION_HEADER data_section = { 
		.Name = ".data",
		.Misc.VirtualSize = data_size, 
		.VirtualAddress   = align(idata_section.VirtualAddress + idata_section.SizeOfRawData, section_alignment), 
		.SizeOfRawData    = align(data_size, file_alignment), 
		.PointerToRawData = idata_section.PointerToRawData + idata_section.SizeOfRawData, 
		.Characteristics  = IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA,
	}; 
	
	
	//~
	// Creating and filling .idata section
	//
	
	char *idata = calloc(idata_size, 1); // idata buffer to fill
	
	int descriptors_base_address    = idata_section_address;
	int int_table_base_address      = descriptors_base_address + descriptors_size;
	int function_names_base_address = int_table_base_address + int_tables_size;
	int iat_table_base_address      = function_names_base_address + function_names_size;
	int module_names_base_address   = iat_table_base_address + int_tables_size;
	
	size_t *int_tables = (size_t *)(idata + descriptors_size); 
	char *function_names = (char *)int_tables + int_tables_size;
	size_t *iat_tables = (size_t *)(function_names + function_names_size);
	char *module_names   = (char *)iat_tables + int_tables_size;
	
	int int_tables_offset = 0, module_names_offset = 0, function_names_offset = 0; 
	for (int i = 0; i < entries_count; i++) {
		
		// filling the descriptors 
		((IMAGE_IMPORT_DESCRIPTOR *)idata)[i] = (IMAGE_IMPORT_DESCRIPTOR){
			.OriginalFirstThunk = int_table_base_address    + int_tables_offset,
			.Name               = module_names_base_address + module_names_offset,
			.FirstThunk         = iat_table_base_address    + int_tables_offset
		};
		
		// filling function names, int table and iat table
		char **fnames = entries[i].function_names, **fnames_start = fnames;
		for (; *fnames; fnames++) {
			int copy_size = (int)strlen(*fnames)+1;
			// NOTE(ziv): firstb two bytes are used for 'HINT'
			memcpy(function_names + function_names_offset + 2, *fnames, copy_size); 
			// fill the function's addresses into the INT and IAT tables. 
			*int_tables++ = *iat_tables++ = (size_t)function_names_base_address + function_names_offset;
			function_names_offset += copy_size + 2; 
		}
		int_tables_offset += (int)(sizeof(size_t) * ((fnames-fnames_start)+1)); 
		int_tables++; iat_tables++; // for NULL address
		
		// filling module names 
		size_t copy_size = strlen(entries[i].dll_name)+1; 
		memcpy((void *)(module_names + module_names_offset), (void *)entries[i].dll_name, copy_size);
		module_names_offset += (int)copy_size;
	}
	
	//~
	// Creating final executable
	//
	
	size_t executable_size = nt_headers.OptionalHeader.SizeOfHeaders + 
		text_section.SizeOfRawData + idata_section.SizeOfRawData + data_section.SizeOfRawData;
	char *executable = LocalAlloc(LMEM_FIXED, executable_size), *pexe = executable;
	
	MOVE(pexe, image_stub_and_signiture, sizeof(image_stub_and_signiture)); 
	MOVE(pexe, &nt_headers,    sizeof(nt_headers));
	MOVE(pexe, &text_section,  sizeof(text_section));
	MOVE(pexe, &idata_section, sizeof(idata_section));
	MOVE(pexe, &data_section,  sizeof(data_section)); pexe += nt_headers.OptionalHeader.SizeOfHeaders - sizeof_headers_unaligned;
	MOVE(pexe, code,  code_size);  pexe += text_section.SizeOfRawData  - code_size;
	MOVE(pexe, idata, idata_size); pexe += idata_section.SizeOfRawData - idata_size;
	MOVE(pexe, data,  data_size);  pexe += data_section.SizeOfRawData  - data_size;
	
	HANDLE handle = CreateFileA(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	Assert(handle != INVALID_HANDLE_VALUE, "Invalid handle, error 0x%x", (int)GetLastError());
	
	DWORD bytes_written;
	BOOL result =  WriteFile(handle, executable, (int)executable_size, &bytes_written, NULL);
	Assert(result, "Couldn't write to file, error 0x%x", (int)GetLastError());
	
	CloseHandle(handle);
	LocalFree(executable); 
	
	return 0;
}