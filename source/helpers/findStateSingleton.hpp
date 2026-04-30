#pragma once
#include "../util.h"

namespace StateSingletonFinder {

struct AdrpResult {
    u8 regIndex;
    u64 targetPage;
};

struct LdrResult {
    u8 destReg;
    u8 baseReg;
    u64 offset;
};

namespace { // anonymous
    constexpr u64 READ_BUFFER_SIZE = 0x1000;

    // Sanity Check: ADRP mask is 0x1F000000, expected result 0x10000000
    // Bits 31: 1, 30-29: immlo, 28-24: 10000, 23-5: immhi, 4-0: Rd
    bool _isADRP(u32 insn) {
        return (insn & 0x9F000000) == 0x90000000;
    }

    // Sanity Check: LDR (imm, 64-bit) mask 0xFFC00000, expected 0xF9400000
    bool _isLDRx(u32 insn) {
        return (insn & 0xFFC00000) == 0xF9400000;
    }

    u64 _findInstructionInMain(u32 instruction) {
        DmntCheatProcessMetadata metadata;
        dmntchtGetCheatProcessMetadata(&metadata);
        util::PrintToNXLink("MainAdress: 0x%lX\n", metadata.main_nso_extents.base);

        static u32 buffer[READ_BUFFER_SIZE / sizeof(u32)];
        for (u64 offset = 0; offset < metadata.main_nso_extents.size; offset += sizeof(buffer)) {
            if (R_FAILED(dmntchtReadCheatProcessMemory(metadata.main_nso_extents.base + offset, buffer, sizeof(buffer))))
                continue;
            util::PrintToNXLink("Read main segment at offset 0x%lX\n", offset);

            for (size_t i = 0; i < (sizeof(buffer) / sizeof(u32)); i++) {
                if (buffer[i] == instruction) {
                    u64 addressOffset = offset + (i * sizeof(u32));
                    u64 address = metadata.main_nso_extents.base + addressOffset;
                    util::PrintToNXLink("Found instruction at offset 0x%lX (address: 0x%lX)\n", addressOffset, address);
                    // util::PrintToNXLink("Compare (singleton address): 0x%lx (address: 0x%lX)\n", 0x5474040, metadata.main_nso_extents.base + 0x5474040);
                    return address;
                }
            }
        }
        return 0;
    }

    static AdrpResult _decodeADRP(u64 pc, u32 instruction) {
        u8 rd = instruction & 0x1F;
        s64 immlo = (instruction >> 29) & 0x3;
        s64 immhi = (instruction >> 5) & 0x7FFFF;
        s64 imm = (immhi << 2) | immlo;

        // Sign extend 21-bit to 64-bit
        if (imm & (1 << 20)) {
            imm |= ~0x1FFFFF;
        }

        u64 pageBase = pc & ~0xFFFull;
        u64 targetPage = pageBase + (imm << 12);

        return { rd, targetPage };
    }

    static LdrResult _decodeLDR(u32 instruction) {
        u8 rt = instruction & 0x1F;
        u8 rn = (instruction >> 5) & 0x1F;
        u64 imm12 = (instruction >> 10) & 0xFFF;
        u64 offset = imm12 * 8; // Scale for 64-bit
        return { rt, rn, offset };
    }
} // end anonymous namespace

Result findStateSingleton(u64 *outAddress) {
    if (!outAddress) return 0xFFFFFFFF; // invalid argument

    // this is just some unique instruction I found in the State's initializer/constructor.
    u32 const stateInitInstruction = 0xd2c00748; // mov 8,#0x3a00000000
    u64 address = _findInstructionInMain(stateInitInstruction);
    if (address == 0) return 1;

    // offset to where the State singleton is loaded from in the State's accessor function.
    // for 3.0.3:
    //       71025ee1c0 36 74 01 d0     adrp       x22,0x7105474000
    //       71025ee1c4 d4 22 40 f9     ldr        x20,[x22, #0x40]=>g_StatePointer
    u64 const stateAccessorOffset = 0xF8;
    u64 adrpAddress = address - stateAccessorOffset;
    u32 adrpInstruction;
    dmntchtReadCheatProcessMemory(adrpAddress, &adrpInstruction, sizeof(adrpInstruction));

    util::PrintToNXLink("ADRP PC: 0x%08lX\n", adrpAddress);
    util::PrintToNXLink("ADRP Instruction: 0x%08X\n", adrpInstruction);

    if (!_isADRP(adrpInstruction)) return 2;
    AdrpResult adrpRes = _decodeADRP(adrpAddress, adrpInstruction);
    u64 ldrAddress = adrpAddress + 4;
    u32 ldrInstruction;
    dmntchtReadCheatProcessMemory(ldrAddress, &ldrInstruction, sizeof(ldrInstruction));

    util::PrintToNXLink("LDR PC: 0x%08lX\n", ldrAddress);
    util::PrintToNXLink("LDR Instruction: 0x%08X\n", ldrInstruction);

    if (!_isLDRx(ldrInstruction)) return 3;
    LdrResult ldrRes = _decodeLDR(ldrInstruction);

    // verify the registers used are the same, or else the pattern has changed
    if (ldrRes.baseReg != adrpRes.regIndex) return 4;
    *outAddress = adrpRes.targetPage + ldrRes.offset;
    return 0;
}

} // end namespace StateSingletonFinder