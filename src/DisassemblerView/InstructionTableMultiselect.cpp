#include "DisassemblerView/InstructionTableMultiselect.hpp"

#include "DisassemblerView/Core.hpp"
#include "DisassemblerView/InstructionTable.hpp"
#include "Z80Disassembler.h"

#include <sstream>  // std::ostringstream

static const auto& parsed = DisassemblerView::GetDisassemblerParsedGeneral();



void InstructionTableMultiselect::WriteSelectionToClipboard(const std::vector<display_instr>& display_instrs)
{
    std::ostringstream clipboard;

    for (int i = 0; i < display_instrs.size(); ++i) {
        if (!Contains(GetStorageIdFromIndex(i)))
            continue;

        const auto& display = display_instrs[i];

        if (display.is_separator) {
            clipboard << '\n';
            continue;
        }

        const auto& instr = parsed.at(display.address);
        std::ostringstream line;
        line << to_hex(instr.address, false) << ": "
                  << z80_get_mnemonic_from_index(instr.type) << " "
                  << instr.operands[0]
                  << instr.operands[1];
        clipboard << line.str() << "\n";
    }

    ImGui::SetClipboardText(clipboard.str().c_str());
}
