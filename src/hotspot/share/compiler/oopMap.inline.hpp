/*
 * Copyright (c) 1998, 2015, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_COMPILER_OOPMAP_INLINE_HPP
#define SHARE_VM_COMPILER_OOPMAP_INLINE_HPP

#include "gc/shared/collectedHeap.hpp"
#include "gc/shared/gc_globals.hpp"
#include "memory/universe.hpp"
#include "oops/compressedOops.hpp"

inline const ImmutableOopMap* ImmutableOopMapSet::find_map_at_slot(int slot, int pc_offset) const {
  assert(slot >= 0 && slot < _count, "bounds count: %d slot: %d", _count, slot);
  ImmutableOopMapPair* pairs = get_pairs();
  ImmutableOopMapPair* last = &pairs[slot];
  assert(last->pc_offset() == pc_offset, "oopmap not found");
  return last->get_from(this);
}

inline const ImmutableOopMap* ImmutableOopMapPair::get_from(const ImmutableOopMapSet* set) const {
  return set->oopmap_at_offset(_oopmap_offset);
}

inline bool SkipNullValue::should_skip(oop val) {
  return val == (oop)NULL || CompressedOops::is_base(val);
}

template <typename OopFnT, typename DerivedOopFnT, typename ValueFilterT>
template <typename RegisterMapT>
void OopMapDo<OopFnT, DerivedOopFnT, ValueFilterT>::iterate_oops_do(const frame *fr, const RegisterMapT *reg_map, const ImmutableOopMap* oopmap) {
  NOT_PRODUCT(if (TraceCodeBlobStacks) OopMapSet::trace_codeblob_maps(fr, reg_map->as_RegisterMap());)
  assert (fr != NULL, "");

  // handle derived pointers first (otherwise base pointer may be
  // changed before derived pointer offset has been collected)
  if (_derived_oop_fn != nullptr) {
    for (OopMapStream oms(oopmap); !oms.is_done(); oms.next()) {
      OopMapValue omv = oms.current();
      if (omv.type() != OopMapValue::derived_oop_value)
        continue;
        
  #ifndef COMPILER2
      COMPILER1_PRESENT(ShouldNotReachHere();)
  #if INCLUDE_JVMCI
      if (UseJVMCICompiler) {
        ShouldNotReachHere();
      }
  #endif
  #endif // !COMPILER2

      address loc = fr->oopmapreg_to_location(omv.reg(), reg_map);

      DEBUG_ONLY(if (reg_map->should_skip_missing()) continue;)
      guarantee(loc != NULL, "missing saved register");
      derived_pointer* derived_loc = (derived_pointer*)loc;
      oop* base_loc = fr->oopmapreg_to_oop_location(omv.content_reg(), reg_map);
      // Ignore NULL oops and decoded NULL narrow oops which
      // equal to CompressedOops::base() when a narrow oop
      // implicit null check is used in compiled code.
      // The narrow_oop_base could be NULL or be the address
      // of the page below heap depending on compressed oops mode.
      if (base_loc != NULL && *base_loc != (oop)NULL && !CompressedOops::is_base(*base_loc)) {
        _derived_oop_fn->do_derived_oop(base_loc, derived_loc);
      }
    }
  }

  // We want coop and oop oop_types
  if (_oop_fn != nullptr) {
    for (OopMapStream oms(oopmap); !oms.is_done(); oms.next()) {
      OopMapValue omv = oms.current();
      if (omv.type() != OopMapValue::oop_value && omv.type() != OopMapValue::narrowoop_value)
        continue;
      oop* loc = fr->oopmapreg_to_oop_location(omv.reg(),reg_map);
      // It should be an error if no location can be found for a
      // register mentioned as contained an oop of some kind.  Maybe
      // this was allowed previously because value_value items might
      // be missing?
#ifdef ASSERT
      if (loc == NULL) {
        if (reg_map->should_skip_missing())
          continue;
        VMReg reg = omv.reg();
        tty->print_cr("missing saved register: reg: " INTPTR_FORMAT " %s loc: %p", reg->value(), reg->name(), loc);
        fr->print_on(tty);
      }
#endif
      guarantee(loc != NULL, "missing saved register");
      if ( omv.type() == OopMapValue::oop_value ) {
        oop val = *loc;
        if (ValueFilterT::should_skip(val)) { // TODO: UGLY (basically used to decide if we're freezing/thawing continuation)
          // Ignore NULL oops and decoded NULL narrow oops which
          // equal to CompressedOops::base() when a narrow oop
          // implicit null check is used in compiled code.
          // The narrow_oop_base could be NULL or be the address
          // of the page below heap depending on compressed oops mode.
          continue;
        }
        _oop_fn->do_oop(loc);
      } else if ( omv.type() == OopMapValue::narrowoop_value ) {
        narrowOop *nl = (narrowOop*)loc;
#ifndef VM_LITTLE_ENDIAN
        VMReg vmReg = omv.reg();
        if (!vmReg->is_stack()) {
          // compressed oops in registers only take up 4 bytes of an
          // 8 byte register but they are in the wrong part of the
          // word so adjust loc to point at the right place.
          nl = (narrowOop*)((address)nl + 4);
        }
#endif
        _oop_fn->do_oop(nl);
      }
    }
  }
}


template <typename OopFnT, typename DerivedOopFnT, typename ValueFilterT>
template <typename RegisterMapT>
void OopMapDo<OopFnT, DerivedOopFnT, ValueFilterT>::oops_do(const frame *fr, const RegisterMapT *reg_map, const ImmutableOopMap* oopmap) {
  iterate_oops_do(fr, reg_map, oopmap);
}

#endif

