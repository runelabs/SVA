//===-- X86MCAsmInfo.cpp - X86 asm properties -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the X86MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "X86MCAsmInfo.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ELF.h"
using namespace llvm;

enum AsmWriterFlavorTy {
  // Note: This numbering has to match the GCC assembler dialects for inline
  // asm alternatives to work right.
  ATT = 0, Intel = 1
};

static cl::opt<AsmWriterFlavorTy>
AsmWriterFlavor("x86-asm-syntax", cl::init(ATT),
  cl::desc("Choose style of code to emit from X86 backend:"),
  cl::values(clEnumValN(ATT,   "att",   "Emit AT&T-style assembly"),
             clEnumValN(Intel, "intel", "Emit Intel-style assembly"),
             clEnumValEnd));


static const char *const x86_asm_table[] = {
  "{si}", "S",
  "{di}", "D",
  "{ax}", "a",
  "{cx}", "c",
  "{memory}", "memory",
  "{flags}", "",
  "{dirflag}", "",
  "{fpsr}", "",
  "{fpcr}", "",
  "{cc}", "cc",
  0,0};

void X86MCAsmInfoDarwin::anchor() { }

X86MCAsmInfoDarwin::X86MCAsmInfoDarwin(const Triple &T) {
  bool is64Bit = T.getArch() == Triple::x86_64;
  if (is64Bit)
    PointerSize = 8;

  AsmTransCBE = x86_asm_table;
  AssemblerDialect = AsmWriterFlavor;

  TextAlignFillValue = 0x90;

  if (!is64Bit)
    Data64bitsDirective = 0;       // we can't emit a 64-bit unit

  // Use ## as a comment string so that .s files generated by llvm can go
  // through the GCC preprocessor without causing an error.  This is needed
  // because "clang foo.s" runs the C preprocessor, which is usually reserved
  // for .S files on other systems.  Perhaps this is because the file system
  // wasn't always case preserving or something.
  CommentString = "##";
  PCSymbol = ".";

  SupportsDebugInformation = true;
  DwarfUsesInlineInfoSection = true;

  // Exceptions handling
  ExceptionsType = ExceptionHandling::DwarfCFI;
}

X86_64MCAsmInfoDarwin::X86_64MCAsmInfoDarwin(const Triple &Triple)
  : X86MCAsmInfoDarwin(Triple) {
}

void X86ELFMCAsmInfo::anchor() { }

X86ELFMCAsmInfo::X86ELFMCAsmInfo(const Triple &T) {
  if (T.getArch() == Triple::x86_64)
    PointerSize = 8;

  AsmTransCBE = x86_asm_table;
  AssemblerDialect = AsmWriterFlavor;

  TextAlignFillValue = 0x90;

  PrivateGlobalPrefix = ".L";
  WeakRefDirective = "\t.weak\t";
  PCSymbol = ".";

  // Set up DWARF directives
  HasLEB128 = true;  // Target asm supports leb128 directives (little-endian)

  // Debug Information
  SupportsDebugInformation = true;

  // Exceptions handling
  ExceptionsType = ExceptionHandling::DwarfCFI;

  // OpenBSD has buggy support for .quad in 32-bit mode, just split into two
  // .words.
  if (T.getOS() == Triple::OpenBSD && T.getArch() == Triple::x86)
    Data64bitsDirective = 0;
}

const MCExpr *
X86_64MCAsmInfoDarwin::getExprForPersonalitySymbol(const MCSymbol *Sym,
                                                   unsigned Encoding,
                                                   MCStreamer &Streamer) const {
  MCContext &Context = Streamer.getContext();
  const MCExpr *Res =
    MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_GOTPCREL, Context);
  const MCExpr *Four = MCConstantExpr::Create(4, Context);
  return MCBinaryExpr::CreateAdd(Res, Four, Context);
}

const MCSection *X86ELFMCAsmInfo::
getNonexecutableStackSection(MCContext &Ctx) const {
  return Ctx.getELFSection(".note.GNU-stack", ELF::SHT_PROGBITS,
                           0, SectionKind::getMetadata());
}

void X86MCAsmInfoMicrosoft::anchor() { }

X86MCAsmInfoMicrosoft::X86MCAsmInfoMicrosoft(const Triple &Triple) {
  if (Triple.getArch() == Triple::x86_64) {
    GlobalPrefix = "";
    PrivateGlobalPrefix = ".L";
  }

  AsmTransCBE = x86_asm_table;
  AssemblerDialect = AsmWriterFlavor;

  TextAlignFillValue = 0x90;
}

void X86MCAsmInfoGNUCOFF::anchor() { }

X86MCAsmInfoGNUCOFF::X86MCAsmInfoGNUCOFF(const Triple &Triple) {
  if (Triple.getArch() == Triple::x86_64) {
    GlobalPrefix = "";
    PrivateGlobalPrefix = ".L";
  }

  AsmTransCBE = x86_asm_table;
  AssemblerDialect = AsmWriterFlavor;

  TextAlignFillValue = 0x90;

  // Exceptions handling
  ExceptionsType = ExceptionHandling::DwarfCFI;
}