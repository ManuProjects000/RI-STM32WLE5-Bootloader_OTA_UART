import serial
import struct
import time
import zlib
import binascii

# === CONFIGURACIÓN ===
PORT = "COM4"         # Puerto serie de tu placa
#BAUDRATE = 57600
BAUDRATE = 115200

ETX_OTA_SOF  	    =   '$'
ETX_OTA_SALTO_LINEA =	0x0D
ETX_OTA_FIN_LINEA	=	0x0A  

ETX_OTA_ACK  		=	0x00
ETX_OTA_NACK 		=	0x01

ETX_OTA_PACKET_TYPE_SECTOR =	1	
ETX_OTA_DATA_SECTOR		   =	4	

#CAMBIE EL TAMANIO DE LOS PAQUETES. FIJARME EN LA APP EL TAMANIO DE DATA
#DE PAQUETES
ETX_OTA_DATA_MAX_SIZE = 256

ETX_OTA_DATA_OVERHEAD = 10

ETX_OTA_PACKET_MAX_SIZE = ( ETX_OTA_DATA_MAX_SIZE + ETX_OTA_DATA_OVERHEAD )

ETX_OTA_PACKET_TYPE_CMD       		= 0  
ETX_OTA_PACKET_TYPE_DATA      		= 1 
ETX_OTA_PACKET_TYPE_HEADER    		= 2 
ETX_OTA_PACKET_TYPE_BULK_HEADER     = 3 
ETX_OTA_PACKET_TYPE_RESPONSE  		= 4 

ETX_OTA_CMD_START = 0
ETX_OTA_CMD_END   = 1

PAGE_SIZE = 2048
PACKETS_PER_PAGE = PAGE_SIZE/ETX_OTA_DATA_MAX_SIZE

crc_table = [
		0x00000000, 0x04C11DB7,  0x09823B6E, 0x0D4326D9,   0x130476DC, 0x17C56B6B,  0x1A864DB2, 0x1E475005,
        0x2608EDB8, 0x22C9F00F,  0x2F8AD6D6, 0x2B4BCB61,   0x350C9B64, 0x31CD86D3,  0x3C8EA00A, 0x384FBDBD,
        0x4C11DB70, 0x48D0C6C7,  0x4593E01E, 0x4152FDA9,   0x5F15ADAC, 0x5BD4B01B,  0x569796C2, 0x52568B75,
        0x6A1936C8, 0x6ED82B7F,  0x639B0DA6, 0x675A1011,   0x791D4014, 0x7DDC5DA3,  0x709F7B7A, 0x745E66CD,
        0x9823B6E0, 0x9CE2AB57,  0x91A18D8E, 0x95609039,   0x8B27C03C, 0x8FE6DD8B,  0x82A5FB52, 0x8664E6E5,
        0xBE2B5B58, 0xBAEA46EF,  0xB7A96036, 0xB3687D81,   0xAD2F2D84, 0xA9EE3033,  0xA4AD16EA, 0xA06C0B5D,
        0xD4326D90, 0xD0F37027,  0xDDB056FE, 0xD9714B49,   0xC7361B4C, 0xC3F706FB,  0xCEB42022, 0xCA753D95,
        0xF23A8028, 0xF6FB9D9F,  0xFBB8BB46, 0xFF79A6F1,   0xE13EF6F4, 0xE5FFEB43,  0xE8BCCD9A, 0xEC7DD02D,
        0x34867077, 0x30476DC0,  0x3D044B19, 0x39C556AE,   0x278206AB, 0x23431B1C,  0x2E003DC5, 0x2AC12072,
        0x128E9DCF, 0x164F8078,  0x1B0CA6A1, 0x1FCDBB16,   0x018AEB13, 0x054BF6A4,  0x0808D07D, 0x0CC9CDCA,
        0x7897AB07, 0x7C56B6B0,  0x71159069, 0x75D48DDE,   0x6B93DDDB, 0x6F52C06C,  0x6211E6B5, 0x66D0FB02,
        0x5E9F46BF, 0x5A5E5B08,  0x571D7DD1, 0x53DC6066,   0x4D9B3063, 0x495A2DD4,  0x44190B0D, 0x40D816BA,
        0xACA5C697, 0xA864DB20,  0xA527FDF9, 0xA1E6E04E,   0xBFA1B04B, 0xBB60ADFC,  0xB6238B25, 0xB2E29692,
        0x8AAD2B2F, 0x8E6C3698,  0x832F1041, 0x87EE0DF6,   0x99A95DF3, 0x9D684044,  0x902B669D, 0x94EA7B2A,
        0xE0B41DE7, 0xE4750050,  0xE9362689, 0xEDF73B3E,   0xF3B06B3B, 0xF771768C,  0xFA325055, 0xFEF34DE2,
        0xC6BCF05F, 0xC27DEDE8,  0xCF3ECB31, 0xCBFFD686,   0xD5B88683, 0xD1799B34,  0xDC3ABDED, 0xD8FBA05A,
        0x690CE0EE, 0x6DCDFD59,  0x608EDB80, 0x644FC637,   0x7A089632, 0x7EC98B85,  0x738AAD5C, 0x774BB0EB,
        0x4F040D56, 0x4BC510E1,  0x46863638, 0x42472B8F,   0x5C007B8A, 0x58C1663D,  0x558240E4, 0x51435D53,
        0x251D3B9E, 0x21DC2629,  0x2C9F00F0, 0x285E1D47,   0x36194D42, 0x32D850F5,  0x3F9B762C, 0x3B5A6B9B,
        0x0315D626, 0x07D4CB91,  0x0A97ED48, 0x0E56F0FF,   0x1011A0FA, 0x14D0BD4D,  0x19939B94, 0x1D528623,
        0xF12F560E, 0xF5EE4BB9,  0xF8AD6D60, 0xFC6C70D7,   0xE22B20D2, 0xE6EA3D65,  0xEBA91BBC, 0xEF68060B,
        0xD727BBB6, 0xD3E6A601,  0xDEA580D8, 0xDA649D6F,   0xC423CD6A, 0xC0E2D0DD,  0xCDA1F604, 0xC960EBB3,
        0xBD3E8D7E, 0xB9FF90C9,  0xB4BCB610, 0xB07DABA7,   0xAE3AFBA2, 0xAAFBE615,  0xA7B8C0CC, 0xA379DD7B,
        0x9B3660C6, 0x9FF77D71,  0x92B45BA8, 0x9675461F,   0x8832161A, 0x8CF30BAD,  0x81B02D74, 0x857130C3,
        0x5D8A9099, 0x594B8D2E,  0x5408ABF7, 0x50C9B640,   0x4E8EE645, 0x4A4FFBF2,  0x470CDD2B, 0x43CDC09C,
        0x7B827D21, 0x7F436096,  0x7200464F, 0x76C15BF8,   0x68860BFD, 0x6C47164A,  0x61043093, 0x65C52D24,
        0x119B4BE9, 0x155A565E,  0x18197087, 0x1CD86D30,   0x029F3D35, 0x065E2082,  0x0B1D065B, 0x0FDC1BEC,
        0x3793A651, 0x3352BBE6,  0x3E119D3F, 0x3AD08088,   0x2497D08D, 0x2056CD3A,  0x2D15EBE3, 0x29D4F654,
        0xC5A92679, 0xC1683BCE,  0xCC2B1D17, 0xC8EA00A0,   0xD6AD50A5, 0xD26C4D12,  0xDF2F6BCB, 0xDBEE767C,
        0xE3A1CBC1, 0xE760D676,  0xEA23F0AF, 0xEEE2ED18,   0xF0A5BD1D, 0xF464A0AA,  0xF9278673, 0xFDE69BC4,
        0x89B8FD09, 0x8D79E0BE,  0x803AC667, 0x84FBDBD0,   0x9ABC8BD5, 0x9E7D9662,  0x933EB0BB, 0x97FFAD0C,
        0xAFB010B1, 0xAB710D06,  0xA6322BDF, 0xA2F33668,   0xBCB4666D, 0xB8757BDA,  0xB5365D03, 0xB1F740B4
]

def read_packet(ser, max_size=1024):
    packet = bytearray()
    last_byte = None
    while True:
        b = ser.read(1)
        if not b:
            continue
        packet.append(b[0])
        if last_byte == 0x0D and b[0] == 0x0A:
            break
        last_byte = b[0]
        if len(packet) >= max_size:
            break
    return bytes(packet)

# Función para calcular CRC32
def calculate_flash_crc(data_bytes: bytes) -> int:
    """
    Calcula el CRC de todo el firmware (como en calculateFlashCRC de C).
    data_bytes: objeto bytes o bytearray con el firmware completo.
    """
    crc = 0xFFFFFFFF
    total_len = len(data_bytes)

    # Procesar bloques de 4 bytes (uint32_t)
    num_words = total_len // 4
    remainder = total_len % 4

    for i in range(num_words):
        val = int.from_bytes(data_bytes[i*4:(i+1)*4], byteorder="big")
        for _ in range(4):
            byte = (val >> 24) & 0xFF
            table_index = ((crc >> 24) ^ byte) & 0xFF
            crc = ((crc << 8) & 0xFFFFFFFF) ^ crc_table[table_index]
            val = (val << 8) & 0xFFFFFFFF

    # Procesar los bytes sobrantes al final (si los hay)
    if remainder:
        val = int.from_bytes(data_bytes[-remainder:] + b'\x00'*(4-remainder), byteorder="big")
        for _ in range(remainder):
            byte = (val >> 24) & 0xFF
            table_index = ((crc >> 24) ^ byte) & 0xFF
            crc = ((crc << 8) & 0xFFFFFFFF) ^ crc_table[table_index]
            val = (val << 8) & 0xFFFFFFFF

    return crc & 0xFFFFFFFF

def calculate_crc_word(data_word: bytes) -> int:
    """
    Calcula el CRC de un uint32_t como calculateCRC en C.
    data_word: entero de 32 bits.
    """
    crc = 0xFFFFFFFF

    for i in range(0, 16, 4):
        # Convertir 4 bytes a uint32_t big-endian
        word = int.from_bytes(data_word[i:i+4], byteorder='big') & 0xFFFFFFFF
        val = word
        for _ in range(4):
            byte = (val >> 24) & 0xFF
            table_index = ((crc >> 24) ^ byte) & 0xFF
            crc = ((crc << 8) & 0xFFFFFFFF) ^ crc_table[table_index]
            val = (val << 8) & 0xFFFFFFFF

    return crc & 0xFFFFFFFF

def calculate_crc_word_datapack(data_word: bytes) -> int:
    """
    Calcula el CRC de un uint32_t como calculateCRC en C.
    data_word: entero de 32 bits.
    """
    crc = 0xFFFFFFFF

    for i in range(0, ETX_OTA_DATA_MAX_SIZE, 4):
        # Convertir 4 bytes a uint32_t big-endian
        word = int.from_bytes(data_word[i:i+4], byteorder='big') & 0xFFFFFFFF
        val = word
        for _ in range(4):
            byte = (val >> 24) & 0xFF
            table_index = ((crc >> 24) ^ byte) & 0xFF
            crc = ((crc << 8) & 0xFFFFFFFF) ^ crc_table[table_index]
            val = (val << 8) & 0xFFFFFFFF

    return crc & 0xFFFFFFFF

def calculate_crc_command(command_byte: int) -> int:
    """
    Calcula el CRC de un paquete command de 1 byte (por ejemplo 0x01)
    Retorna un CRC de 4 bytes (uint32_t)
    """
    crc = 0xFFFFFFFF

    # Convertimos el byte en 4 bytes big-endian, rellenando con ceros
    val = (command_byte & 0xFF) << 24  # equivalente a b'\x01\x00\x00\x00'

    for _ in range(4):
        byte = (val >> 24) & 0xFF
        table_index = ((crc >> 24) ^ byte) & 0xFF
        crc = ((crc << 8) & 0xFFFFFFFF) ^ crc_table[table_index]
        val = (val << 8) & 0xFFFFFFFF

    return crc & 0xFFFFFFFF

def calculate_crc_page(page):
    """Recorre la página cada 16 bytes y hace un CRC acumulativo con XOR"""
    crc = 0xFFFFFFFF
    for offset in range(0, len(page), 16):
        block = page[offset:offset+16]
        if len(block) == 16:
            crc ^= calculate_crc_word(block)
    return crc & 0xFFFFFFFF

def hex_dump(packet_type, length, data, bytes_per_line=16):
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i+bytes_per_line]
        hex_bytes = ' '.join(f"{b:02X}" for b in chunk)
        ascii_bytes = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in chunk)
        print(f"{i:08X} {ETX_OTA_SOF} {packet_type} {length:02X} {hex_bytes:<48} ")

# Función para crear paquetes
def make_packet_header(firmware):
    hdata = struct.pack("<I I I I",
                    len(firmware),               # tamaño firmware
                    calculate_flash_crc(firmware),# CRC firmware
                    0x00000000,             # campo reservado 1
                    0x00000000)             # campo reservado 2
    packet_type = ETX_OTA_PACKET_TYPE_HEADER
    length = len(hdata)
    crc = calculate_crc_word(hdata)

    packet = struct.pack(
        "<c B H",                    # SOF (char), Type (uint8), Length (uint16 little-endian)
        ETX_OTA_SOF.encode(),        # SOF '$'
        packet_type,                 # TYPE
        length                      # LENGTH (2 bytes)
    )
    packet += hdata                 # DATA
    packet += struct.pack("<I", crc) # CRC (4 bytes little-endian)
    packet += struct.pack("BB", ETX_OTA_SALTO_LINEA, ETX_OTA_FIN_LINEA)  # SALTO_LINEA + FIN_LINEA
    
    return packet

def make_packet_bulk_header(bulk):
    packet_type = ETX_OTA_PACKET_TYPE_BULK_HEADER

    # Calcular CRC del bulk (entero de 4 bytes)
    crc_bulk = calculate_flash_crc(bulk)  # devuelve int
    #print(f"CRC bulk: 0x{crc_bulk:08X}")
    # Convertir a bytes para pasarlo al CRC de palabra
    crc_bulk_bytes = struct.pack("<I", crc_bulk)  # 4 bytes, little-endian

    length = 4

    # Calcular CRC sobre esos 4 bytes
    crc = calculate_crc_word(crc_bulk_bytes)

    # Construir paquete
    packet = struct.pack("<c B H", ETX_OTA_SOF.encode(), packet_type, length)
    packet += crc_bulk_bytes             # DATA
    packet += struct.pack("<I", crc)     # CRC de esos 4 bytes
    packet += struct.pack("BB", ETX_OTA_SALTO_LINEA, ETX_OTA_FIN_LINEA)

    return packet

def make_packet_data(chunk, crc, length):
    packet_type = ETX_OTA_PACKET_TYPE_DATA

    packet = struct.pack(
        "<c B H",                # SOF, TYPE, LENGTH
        ETX_OTA_SOF.encode(),
        packet_type,
        length
    )
    packet += chunk    
    packet += struct.pack("<I", crc)  # CRC 4 bytes LE
    packet += struct.pack("BB", ETX_OTA_SALTO_LINEA, ETX_OTA_FIN_LINEA)
    
    return packet

def make_packet_cmd(cmd):
    packet_type = ETX_OTA_PACKET_TYPE_CMD
    length = 1
    crc = calculate_crc_command(cmd)
    
    packet = struct.pack(
        "<c B H",
        ETX_OTA_SOF.encode(),
        packet_type,
        length
    )
    packet += struct.pack("B", cmd)  # CMD (1 byte)
    packet += struct.pack("<I", crc)
    packet += struct.pack("BB", ETX_OTA_SALTO_LINEA, ETX_OTA_FIN_LINEA)
    
    return packet



# Abrir puerto serie
ser = serial.Serial(PORT, BAUDRATE, timeout=1)


# Leer firmware
with open("firmware.bin", "rb") as f:
    firmware = f.read()

# CRC de toda la app
crc_app = calculate_flash_crc(firmware)
print(f"CRC App: 0x{crc_app:08X}")

cant_bulks = len(firmware)/PAGE_SIZE
cant_paq = len(firmware)/ETX_OTA_DATA_MAX_SIZE
print(f"Bulks Totales: {cant_bulks}")
print(f"Paquetes Totales: {cant_paq}")

start_time = time.time()

packet = make_packet_cmd(ETX_OTA_CMD_START)
ser.write(packet)
print(f"Mando CMD: START")

#while ack_event 
while True:
    line = ser.readline().decode(errors="ignore").strip()
    line = line.replace("\x00", "").strip()
    if line == "ACK":
        break
time.sleep(0.5)

# MANDO HEADER
packet = make_packet_header(firmware)
#print(f"HEADER ({len(packet)} bytes): {binascii.hexlify(packet).decode().upper()}")
ser.write(packet)
print(f"Mando HEADER")

#while ack_event 
while True:
    line = ser.readline().decode(errors="ignore").strip()
    if line == "HEADER_OK":
        break
time.sleep(0.1)
#time.sleep(0.55)
# Fragmentar y enviar
# MANDO DATA

####################################################################################
########### PROBAR DE SUMAR  6144  AL OFFSET PARA EVALUAR EL ULTIMO BULK ###########
####################################################################################

offset = 0
packet_count = 0
bulk_count = 0
while offset < len(firmware): # and (packet_count < 128*2):

    if(bulk_count == 0):
        
        bulk_chunk = firmware[offset:offset+PAGE_SIZE]

        bulk_header = make_packet_bulk_header(bulk_chunk) 
        #print(f"BULK HEADER ({len(bulk_header)} bytes): {binascii.hexlify(bulk_header).decode().upper()}")
        
        ser.write(bulk_header)

        #ser.write(bulk_header)
        while True:
            line = ser.readline()  # lee hasta '\n'
            if line:
                print("Micro confirmó que BULK fue procesado.")
                break
        time.sleep(0.1)
        #time.sleep(0.55)
    
    chunk = firmware[offset:offset+ETX_OTA_DATA_MAX_SIZE]
    length_real = len(chunk)
    crc = calculate_crc_word_datapack(chunk)

    packet = make_packet_data(chunk,crc,length_real)
    
    #print(f"Paquete {packet_count}: {binascii.hexlify(packet).decode().upper()}")  
    ser.write(packet)
    
    if(bulk_count < PACKETS_PER_PAGE):
        while True:
            line = ser.readline()  # lee hasta '\n'
            if line:
                print("Micro confirmó que DATA fue procesado.")
                break

    packet_count += 1
    bulk_count += 1
    offset += ETX_OTA_DATA_MAX_SIZE
    time.sleep(0.05)
    #time.sleep(0)
    if(bulk_count == PACKETS_PER_PAGE):
        print(f"Espero confirmacion de escritura de pagina")
        while True:
            line = ser.readline().decode(errors="ignore").strip()
            line = line.replace("\x00", "").strip()
            #print(f"[{line}] -> {repr(line)} (len={len(line)})")
            #print(f"{line}")
            if line == "ACK":
                #print("Pagina escrita correctamente, continuo")
                break
            elif line == "NACK":
                print("Reenvio ultimo bulk")
                offset -= PAGE_SIZE
                packet_count -= PACKETS_PER_PAGE
                break
        bulk_count = 0
        checksum = 0

# Enviar comando END
# MANDO END
time.sleep(0.1)
end_packet = make_packet_cmd(ETX_OTA_CMD_END)
#print(f"END ({len(end_packet)} bytes): {binascii.hexlify(end_packet).decode().upper()}")
ser.write(end_packet)
print("Comando END enviado.")

end_time = time.time()  
elapsed = int(end_time - start_time)  # segundos totales

# Formato hh:mm:ss
formatted = time.strftime("%H:%M:%S", time.gmtime(elapsed))
print(f"Transferencia OTA finalizada. Tiempo: {formatted}")




