def decode_ble_data(data_hex):
    # Convert hex string to bytes
    data_bytes = bytes.fromhex(data_hex)
    
    print(f"Raw data: {data_hex} → {data_bytes.hex()}")
    print("\nByte breakdown:")
    for i, byte in enumerate(data_bytes):
        signed_byte = int.from_bytes([byte], byteorder='big', signed=True)
        print(f"Byte {i}: 0x{byte:02X} ({signed_byte})")
    
    # Protocol structure analysis
    header = data_bytes[0]
    data = data_bytes[1:-2]  # Data bytes (excluding header, checksum, footer)
    checksum_received = data_bytes[-2]
    footer = data_bytes[-1]
    
    # Calculate expected checksum (using unsigned bytes)
    checksum_calculated = sum(data) & 0xFF
    
    print("\nProtocol analysis:")
    print(f"Header: 0x{header:02X} (expected 0xA5)")
    print(f"Data bytes (signed):")
    for i, byte in enumerate(data):
        signed_byte = int.from_bytes([byte], byteorder='big', signed=True)
        print(f"  Byte {i}: 0x{byte:02X} ({signed_byte})")
    print(f"Checksum received: 0x{checksum_received:02X}")
    print(f"Checksum calculated: 0x{checksum_calculated:02X}")
    print(f"Footer: 0x{footer:02X} (expected 0x5A)")

# Example usage
decode_ble_data("a5 7f 7f 81 7f 5a")

# A5 7F 7F 81 7F 5A
#   │   │   │   │
#   │   │   │   └── Footer
#   │   │   └── Checksum (sum of 0x7F + 0x7F + 0x81 = 0x17F → 0x7F)
#   └── Data Bytes (3 bytes)