/**************************************************************************/
/*  export_crypto.cpp                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "export_crypto.h"

String ExportCrypto::get_export_key(String p_key) {
	Vector<uint8_t> key = p_key.hex_decode();
	Vector<uint8_t> buffer;
	buffer.resize(32);
	for (int i = 0; i < buffer.size(); i++) {
		int index = key[i] % 64;
		uint8_t byte = i % 4 == 0 ? RefCounted::_key_cstm[index] : InputEvent::_key_cstm[index];
		buffer.write[i] = byte;
	}
	return String::hex_encode_buffer(buffer.ptr(), buffer.size());
}

uint8_t ExportCrypto::get_key_byte_at_index(int p_index) {
	int index = script_encryption_key_order[p_index] % 64;
	uint8_t byte = p_index % 4 == 0 ? RefCounted::_key_cstm[index] : InputEvent::_key_cstm[index];
	return byte;
}

void ExportCrypto::_bind_methods() {
	ClassDB::bind_static_method("ExportCrypto", D_METHOD("get_export_key", "key"), &ExportCrypto::get_export_key);
}