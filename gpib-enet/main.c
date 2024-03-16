
//
// Information about gpib-enet controller can be found at
// http://docs.eao.hawaii.edu/JCMT/gpib/gpib_getstarted/gpib_getstarted.pdf
//
// The idea for making this program came from 
// https://www.eevblog.com/forum/testgear/gpib-enet-still-usable/25/
//
// The proof for the claim I have found here
// http://docs.eao.hawaii.edu/JCMT/gpib/gpib_appnote1_usingit/gpib_appnote1_usingit.pdf
// under the section "Assigning an IP Address to Your GPIB-ENET"
// I have also found how the ipassign utility works under Appendix C, "IPassign Utility"
// http://docs.eao.hawaii.edu/JCMT/gpib/gpib_getstarted/gpib_getstarted.pdf
//
// To make sure it was alright, I made sure to look into this snapshot
// https://www.cloudshark.org/captures/085306acbc43
// 
// Do note that to avoid name conflicts with raylib and reduce dependencies
// I have taken the definitions and prototypes of most structs, functions, 
// and types and just put it in this file as is. This is not ideal yet required
// for when using raylib. If at some point a need to remove this dependency, I
// have left all the original include files commented out though most can be 
// replaced with a simple include to windows.h
//

// ================================================================================
// Instruction for assigning and IP to a GPIB-ENET device using this utility:
//
// Before using IPassign, make sure that you configure the DIP switches
// properly. Switch 6 should be OFF indicating that you are going to assign a
// protocol address from the network. Switch 5 should be ON indicating that
// you are using IPassign to assign the IP address. Switches 8, 7, and 4
// through 1 should all be OFF. Power-on the GPIB-ENET and run IPasssign.
//
// Wait for 10 seconds until the READY light stops flickering
//
// A steady READY LED verifies that the GPIB-ENET successfully
// received its IP address. As long as the READY LED is flickering,
// the address has not been programmed.
// If GPIB-ENET has recieved its IP address, turn DIP switch 5 OFF and 6 ON.
// If on the other hand, GPIB-ENET has not recieved it's IP, re-run this utility
// ================================================================================

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#define HAVE_REMOTE
#include <pcap.h>

#if _MSC_VER >= 1900
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "raylib.lib")
#endif

#define ArrayLength(x) (sizeof(x)/sizeof(x[0]))
#define big16(x) (((x) >> 8 | (x) << 8) & 0xffff)
#define big8(x)  ((x) & 0xff)


#pragma function(memcpy)
void *memcpy(void *dest, void const *src, size_t size) {
	char *dest_temp = (char *)dest;
	char *src_temp  = (char *)src;
	while (size--) *dest_temp++ = *src_temp++;
	return dest;
}

////////////////////////////////

//#include <winsock2.h>
//#include <iphlpapi.h>

#define MIB_IF_TYPE_ETHERNET 6
#define MAX_ADAPTER_NAME_LENGTH         256
#define MAX_ADAPTER_DESCRIPTION_LENGTH  128
#define MAX_ADAPTER_ADDRESS_LENGTH      8

#define ERROR_BUFFER_OVERFLOW 111L
#define ERROR_SUCCESS         0L
#define NO_ERROR              0L

typedef int UINT, BOOL; 
typedef unsigned char BYTE;
typedef unsigned long DWORD, ULONG, *PULONG; 

#ifdef _MSVC_VER
typedef long    __time32_t;
typedef unsigned long long __time64_t;
#ifdef _USE_32BIT_TIME_T
typedef __time32_t time_t;
#else
typedef __time64_t time_t;
#endif
  #endif

typedef struct {
    char String[4 * 4];
} IP_ADDRESS_STRING, *PIP_ADDRESS_STRING, IP_MASK_STRING, *PIP_MASK_STRING;

typedef struct _IP_ADDR_STRING {
    struct _IP_ADDR_STRING* Next;
    IP_ADDRESS_STRING IpAddress;
    IP_MASK_STRING IpMask;
    DWORD Context;
} IP_ADDR_STRING, *PIP_ADDR_STRING;

typedef struct _IP_ADAPTER_INFO {
    struct _IP_ADAPTER_INFO* Next;
    DWORD ComboIndex;
    char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
    char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    UINT AddressLength;
    BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];
    DWORD Index;
    UINT Type;
    UINT DhcpEnabled;
    PIP_ADDR_STRING CurrentIpAddress;
    IP_ADDR_STRING IpAddressList;
    IP_ADDR_STRING GatewayList;
    IP_ADDR_STRING DhcpServer;
    BOOL HaveWins;
    IP_ADDR_STRING PrimaryWinsServer;
    IP_ADDR_STRING SecondaryWinsServer;
    time_t LeaseObtained;
    time_t LeaseExpires;
} IP_ADAPTER_INFO, *PIP_ADAPTER_INFO;

ULONG __stdcall GetAdaptersInfo(PIP_ADAPTER_INFO AdapterInfo, PULONG SizePointer);

////////////////////////////////

//#include <synchapi.h>
void __stdcall Sleep(DWORD dwMilliseconds);
int __stdcall MessageBoxA(int *handle, char *text, char *capation, unsigned int type); 

////////////////////////////////

// https://en.wikipedia.org/wiki/Ethernet_frame#Ethernet_II
typedef struct {
	char mac_target[6];
		char mac_sender[6];
		short protocol; // rarp = 0x8035
} Ethernet_II_Header; 

// https://uomustansiriyah.edu.iq/media/lectures/5/5_2017_02_12!05_28_00_PM.pdf
typedef struct { 
		short hardware_type; // ethernet = 0x1 
		short protocol_type; // ipv4 = 0x800
		char hardware_size;  // default = 6
		char protocol_size;  // default = 4
		short opcode;        // request - 3, reply - 4
		char sender_mac[6];
		char sender_ip[4];
		char target_mac[6];
		char target_ip[4];
} Rarp_Packet;

typedef struct {
	Ethernet_II_Header header; 
	Rarp_Packet packet;
} Rarp_Frame; 

static void 
send_rarp_message(char target_mac_address[6], char target_ip_address[4], char host_mac_address[6], char host_ip_address[4]) {

	// 
	// Setting up the packet
	//

	// RARP Protocol - https://www.ietf.org/rfc/rfc0903.txt
	Rarp_Frame rf = {
		.header = { 
			.protocol = big16(0x8035) // rarp
		}, 
		.packet = {
			.hardware_type = big16(1),     // ethernet
			.protocol_type = big16(0x800), // ipv4
			.hardware_size = big8(0x6),    // mac length
			.protocol_size = big8(0x4),    // ip length
			.opcode        = big16(0x4)    // reply
		}
	};
	
	memcpy(rf.header.mac_target, target_mac_address, 6);
	memcpy(rf.header.mac_sender, host_mac_address,   6);
	
	memcpy(rf.packet.sender_mac, host_mac_address,   6); 
	memcpy(rf.packet.sender_ip,  host_ip_address,    4); 
	memcpy(rf.packet.target_mac, target_mac_address, 6); 
	memcpy(rf.packet.target_ip,  target_ip_address,  4); 
	
	
#if DEBUG
		// make sure the packet sents is okay
    char *b = (char *)&rf;
    for (int i = 1; i <= sizeof(rf); i++) {
        printf("%02hhx", b[i-1]); 
        if (i % 16 == 0) printf("\n");
    }
    printf("\n");
#endif 
	
	// 
	// Sending the data through an adapter
	//

	pcap_if_t *alldevs, *d;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	
	/* Retrieve the device list on the local machine */
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
			fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
			exit(1);
	}
	d = alldevs; // the first device on the list is the one we will choose
    
	
	// Open the device
	if ( (adhandle= pcap_open(d->name,            // name of the device
														sizeof(Rarp_Frame), // portion of the packet to capture
														PCAP_OPENFLAG_PROMISCUOUS,    // promiscuous mode
														1000,             // read timeout
														NULL,             // authentication on the remote machine
														errbuf            // error buffer
														) ) == NULL)
	{
			fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
			// Free the device list
			pcap_freealldevs(alldevs);
			exit(-1);
	}
    
	printf("\nsending on %s...\n\n", d->description);
	
	// At this point, we don't need any more the device list. Free it
	pcap_freealldevs(alldevs);
    
	//Send down the packet 10 times (some packets might be lost, resending is what IPassign does)
	for (int k = 0; k < 10; k++) {
		if (pcap_sendpacket(adhandle, (char *)&rf, sizeof(rf)) != 0)
		{
			fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(adhandle));
			exit(0);
		}
		Sleep(10);
	}
	
	printf("finished.\n");
}




char *parse_ip(char *, int , char *);

// finds the ip and mac address of the host and sends rarp packet 10 times through ethernet 2 - data layer to assign the ip
void assign_ip_to_gpib_enet(char *, char *); 

////////////////////////////////

#define MAX_INPUT_CHARS 3*6-1

typedef enum Widget_Type{
	WT_MAC_INPUTBOX, 
	WT_IP_INPUTBOX, 
	WT_ASSIGN_BUTTON, 
	WT_EXIT_BUTTON, 
	WT_COUNT,
} Widget_Type;

int main() {
	int screen_width = 580;
	int screen_height = 450;
	
	int font_size = 20, pad = 10;
	Rectangle mac_inputbox = { 50, screen_height - 100, 350 + pad*2, font_size + pad*2 }; 
	int iph = font_size+pad*2;
	Rectangle ip_inputbox = { 50, screen_height - (200+iph+pad), 350 + pad*2, iph }; 
	Rectangle assign_button = { 450, ip_inputbox.y,     100, ip_inputbox.height }; 
	Rectangle exit_button   = { 450, ip_inputbox.y+ 60, 100, ip_inputbox.height }; 
	Rectangle *rs[WT_COUNT] = { &mac_inputbox, &ip_inputbox, &assign_button, &exit_button}; 
	
	char mac_address_string[3*6+2] = "00:80:2F:FF:??:??\0\0";
	char ip_address_string[4*4+2] = "141.141.206.42\0";
	char *inputbox_string[WT_COUNT] = { mac_address_string, ip_address_string, "Assign", "Exit" }; 
	int inputbox_text_length[] = { sizeof(mac_address_string)-3, sizeof(ip_address_string)-4 };
	
	
	int frames_counter = 0;
	int active_widget = -1, hovered_widget = -1;
	
	InitWindow(screen_width, screen_height, "GPIB-ENET Assign IP Address");
	SetTargetFPS(60);
	printf("%d\n", ArrayLength(rs));
	while (!WindowShouldClose()) {
		
		bool collided = false;
		for (int i = 0; i < ArrayLength(rs); i++) {
			if(CheckCollisionPointRec(GetMousePosition(), *rs[i])) {
				hovered_widget = i;
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
					active_widget = hovered_widget;
				}
				collided++;
			}
		}
		if (!collided) {
			hovered_widget = -1;
		}
		
		if (active_widget != -1) {
			
		if (active_widget <= WT_IP_INPUTBOX) {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
				
				int key = GetCharPressed();
				while (key > 0) {
					// NOTE: Only allow keys in range [32..125]
					if ((key >= 32) && (key <= 125) && (inputbox_text_length[active_widget] < MAX_INPUT_CHARS))
					{
						inputbox_string[active_widget][inputbox_text_length[active_widget]] = (char)key;
						inputbox_string[active_widget][inputbox_text_length[active_widget]+1] = '\0';
						inputbox_text_length[active_widget]++;
					}
					key = GetCharPressed();  // Check next character in the queue
				}
				
				if (IsKeyPressed(KEY_BACKSPACE)) {
					inputbox_text_length[active_widget]--;
					if (inputbox_text_length[active_widget] < 0) inputbox_text_length[active_widget] = 0;
					inputbox_string[active_widget][inputbox_text_length[active_widget]] = '\0';
				}
				
				
			}
			else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				if (active_widget == WT_EXIT_BUTTON) {
					return 0;
				}
				
				// here I assume active_widget == AS_ASSIGN_BUTTON
				
				// see if input is valid
				bool valid_input = true;
				char *mac_string = inputbox_string[0];
				for (int i = 0; i < 6 && (mac_string[0] && mac_string[1]); i++, mac_string += 3) {
					if (mac_string[2] == ':' || mac_string[2] == '\0') {
						// turn two hex numbers into a byte
						char l = mac_string[0] | 0x20;
						char r = mac_string[1] | 0x20;
#define is_number(x) ('0' <= (x) && (x) <= '9')
#define is_valid_integer_number(x) (0 <= (x) && (x) <= 9)
#define is_valid_byte_number(x) (is_valid_integer_number(x-'0') || is_valid_integer_number(x-'a')) 
						if (!is_valid_byte_number(l) || !is_valid_byte_number(r)) {
							valid_input = false;
							break;
						}
					}
				}
				if (!valid_input) {
					MessageBoxA(NULL, "Invalid MAC Address", "", 0x10L);
				}
				
				char *ip_string = inputbox_string[1];
				int dot_cnt = 0;
				for (int i = 0; *ip_string && dot_cnt < 4 && i < 4*4+3; i++) {
					if (is_number(*ip_string)) {
						ip_string++;
						continue;
					}
					
					if (*ip_string++ == '.') {
						dot_cnt++;
					}
				}
				
				if (valid_input && ((!is_number(*ip_string) && *ip_string) || (dot_cnt != 3))) {
					valid_input = false;
					MessageBoxA(NULL, "Invalid IP Address", "", 0x10L);
				}
				
				if (valid_input && active_widget == WT_ASSIGN_BUTTON) { 
					int id = MessageBoxA(NULL, 
										 "Make sure all DIP Switches are OFF and switch 5 is ON.\n"\
										 "Power GPIB-ENET on and wait for 10 seconds until the\n" \
										 "self-test has finished (READY LIGHT slow flickering).\n\n" \
										 "Do you wish to continue?"
										,"Assign IP Instructions",
										0x30L | 0x4L);
					
					if (id == 6) { // pressed "Yes"
						
						assign_ip_to_gpib_enet(inputbox_string[0],inputbox_string[1]);
					}
					
					 MessageBoxA(NULL, 
								"A steady READY LED verifies that the GPIB-ENET successfully received its IP address. " \
								"Please verify READY LED, if steady turn DIP switch 5 OFF and 6 ON, otherwise re-run this utility.\n"
								,"Assign IP Instructions", 0x40L);
					}
			}
		}
		else {
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
		}
		

		BeginDrawing();
		ClearBackground(RAYWHITE);
		
		char *center_text = "Assign IP Address utility for the GPIB-ENET";
		DrawText(center_text , screen_width / 2 - MeasureText(center_text, font_size)/2, 60, font_size, BLACK);
		
		DrawText("IP Address (e.g. 154.160.70.2)", ip_inputbox.x, ip_inputbox.y - font_size - 15, font_size, BLACK);
		DrawRectangleLinesEx(ip_inputbox,hovered_widget== 1 ? 2 : 1, (Color){0,0,0,255});
		DrawText(inputbox_string[1], ip_inputbox.x+pad, ip_inputbox.y+pad, font_size , BLACK);
		if (active_widget== 1) {
			// Draw blinking underscore char
				if (((frames_counter/30)%2) == 0) DrawText("_", (int)ip_inputbox.x + 10 + MeasureText(inputbox_string[active_widget], font_size), (int)ip_inputbox.y + 12, font_size, BLACK); 
			}
					
			DrawText("Ethernet Address (e.g. 00:80:2F:FF:??:??)", mac_inputbox.x, mac_inputbox.y - font_size - 15, font_size, BLACK);
			DrawRectangleLinesEx(mac_inputbox,hovered_widget== 0 ? 2 : 1, (Color){0,0,0,255});
			DrawText(inputbox_string[0], mac_inputbox.x+pad, mac_inputbox.y+pad, font_size , BLACK);
		if (active_widget== 0) {
				// Draw blinking underscore char
				if (((frames_counter/30)%2) == 0) DrawText("_", (int)mac_inputbox.x + 10 + MeasureText(inputbox_string[active_widget], font_size), (int)mac_inputbox.y + 12, font_size, BLACK); 
		}
		DrawRectangleLinesEx(assign_button,hovered_widget== 2 ? 2 : 1, (Color){0,0,0,255});
		DrawText(inputbox_string[2], assign_button.x+(assign_button.width - MeasureText(inputbox_string[2], font_size))/2, assign_button.y+pad, font_size , BLACK);
		
		DrawRectangleLinesEx(exit_button, hovered_widget== 3 ? 2 : 1, (Color){0,0,0,255});
		DrawText(inputbox_string[3], exit_button.x+(exit_button.width - MeasureText(inputbox_string[3], font_size))/2, exit_button.y+pad, font_size , BLACK);
		
		EndDrawing();

		frames_counter++;
	}
	CloseWindow();
	
	
    return 0;
}



char *parse_ip(char *out, int out_length, char *ip) {
	if (out_length < 4) return NULL;
	
	char *buf = ip;
	int idx = 0;
	for (; *ip; ip++) {
		if (*ip == '.' || *ip == 0) { 
			*ip = '\0';
			out[idx++] = atoi(buf);
			*ip = '.';
			buf = ++ip;
		}
	}
	out[idx] = atoi(buf);
	return out;
}

void assign_ip_to_gpib_enet(char *target_mac_address_string, char *target_ip_address_string) {
	
	// get mac address and ip of host pc
	char host_mac_address[6], host_ip_address[4];
	{
		
		// try to see whether I need to allocate more space if required
		ULONG out_buf_len = sizeof(IP_ADAPTER_INFO); 
		IP_ADAPTER_INFO *adapter_info = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
		if (GetAdaptersInfo(adapter_info,  &out_buf_len) == ERROR_BUFFER_OVERFLOW) { 
			free(adapter_info); 
			adapter_info = (IP_ADAPTER_INFO *)malloc(out_buf_len);
			if (!adapter_info) {
				printf("Error: Program ran out of memory.\n");
				exit(0);
			}
		}
		
		int err = 0; 
		if ((err = GetAdaptersInfo(adapter_info, &out_buf_len)) != NO_ERROR) { 
			printf("Error: GetAdaptersInfo failed with error %d", err);
			exit(0);
		}
		
		for (; adapter_info; adapter_info = adapter_info->Next) {
			if (adapter_info->Type != MIB_IF_TYPE_ETHERNET) continue;
			
#if DEBUG
			printf("Ethernet Adapter: \t%s\n" \
				   "MAC Address \t%.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n" \
				   "IP Address \t%s\n\n", 
				   adapter_info->AdapterName,
				   adapter_info->Address[0], adapter_info->Address[1], adapter_info->Address[2], adapter_info->Address[3], adapter_info->Address[4], adapter_info->Address[5],
				   adapter_info->IpAddressList.IpAddress.String);
#endif 
			
			for (int i = 0; i < adapter_info->AddressLength && i < 6; i++) { 
				host_mac_address[i] = adapter_info->Address[i];
			}
			
			parse_ip(host_ip_address, ArrayLength(host_ip_address), 
					 adapter_info->IpAddressList.IpAddress.String);
			
			break; // break on the first one found
		}
		
	}
	
	// parse mac address of the adapater and wanted ip address
	char target_mac_address[6], target_ip_address[4];
	{
		for (int i = 0; i < 6 && (target_mac_address_string[0] && target_mac_address_string[1]); i++, target_mac_address_string += 3) {
			if (target_mac_address_string[2] == ':' || target_mac_address_string[2] == '\0') {
				// turn two hex numbers into a byte
				char l = target_mac_address_string[0] | 0x20;
				char r = target_mac_address_string[1] | 0x20;
				l = is_number(l) ? l-'0' : l-'a'+10;
				r = is_number(r) ? r-'0' : r-'a'+10;
				target_mac_address[i] = l << 4 | r;
			}
		}
		
		parse_ip(target_ip_address, ArrayLength(target_ip_address), target_ip_address_string);
	}
	
	send_rarp_message(target_mac_address, target_ip_address, host_mac_address, host_ip_address);
}