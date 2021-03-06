// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Address.h"

#include "../HexCoding.h"
#include "../Hash.h"

#include <TrezorCrypto/base58.h>
#include <TrezorCrypto/ecdsa.h>

using namespace TW::Tezos;

bool Address::isValid(const std::string& string) {
    size_t capacity = 128;
    uint8_t buffer[capacity];
    int size = base58_decode_check(string.data(), HASHER_SHA2D, buffer, (int)capacity);

    // verify prefix
    std::array<std::uint8_t, 3> prefix {6, 161, 159};
    for (size_t i = 0; i< prefix.size(); i++) {
        uint8_t byte = buffer[i];
        if (prefix[i] != byte) {
            return false;
        }
    }
    return size == Address::size;
}

Address::Address(const std::string& string) {
    size_t capacity = 128;
    uint8_t buffer[capacity];
    int size = base58_decode_check(string.data(), HASHER_SHA2D, buffer, (int)capacity);
    if (size != Address::size) {
        throw std::invalid_argument("Invalid address key data");
    }
    std::copy(buffer, buffer + Address::size, bytes.begin());
}

Address::Address(const PublicKey& publicKey) {
    auto publicKeySize = publicKey.isCompressed() ? publicKey.secp256k1Size : publicKey.secp256k1ExtendedSize;
    auto encoded = Data(publicKey.bytes.begin(), publicKey.bytes.begin() + publicKeySize);
    auto hash = Hash::blake2b(encoded, 20);
    auto addressData = Data({6, 161, 159});
    append(addressData, hash);
    if (addressData.size() != Address::size) {
        throw std::invalid_argument("Invalid address key data");
    }
    std::copy(addressData.data(), addressData.data() + Address::size, bytes.begin());
}

std::string Address::string() const {
    size_t size = 0;
    b58enc(nullptr, &size, bytes.data(), Address::size);
    size += 16;

    std::string str(size, ' ');
    base58_encode_check(bytes.data(), Address::size, HASHER_SHA2D, &str[0], size);

    return std::string(str.c_str());
}
