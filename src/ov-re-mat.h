/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#if !defined (octave_matrix_h)
#define octave_matrix_h 1

#include <cstdlib>

#include <iostream>
#include <string>

#include "mx-base.h"
#include "oct-alloc.h"
#include "so-array.h"
#include "str-vec.h"

#include "error.h"
#include "oct-stream.h"
#include "ov-base.h"
#include "ov-base-mat.h"
#include "ov-typeinfo.h"

#include "MatrixType.h"

class Octave_map;
class octave_value_list;

class tree_walker;

// Real matrix values.

class
octave_matrix : public octave_base_matrix<NDArray>
{
public:

  octave_matrix (void)
    : octave_base_matrix<NDArray> () { }

  octave_matrix (const Matrix& m)
    : octave_base_matrix<NDArray> (m) { }

  octave_matrix (const Matrix& m, const MatrixType& t)
    : octave_base_matrix<NDArray> (m, t) { }

  octave_matrix (const NDArray& nda)
    : octave_base_matrix<NDArray> (nda) { }

  octave_matrix (const ArrayN<double>& m)
    : octave_base_matrix<NDArray> (NDArray (m)) { }

  octave_matrix (const DiagMatrix& d)
    : octave_base_matrix<NDArray> (Matrix (d)) { }

  octave_matrix (const RowVector& v)
    : octave_base_matrix<NDArray> (Matrix (v)) { }

  octave_matrix (const ColumnVector& v)
    : octave_base_matrix<NDArray> (Matrix (v)) { }

  octave_matrix (const octave_matrix& m)
    : octave_base_matrix<NDArray> (m) { }

  ~octave_matrix (void) { }

  octave_base_value *clone (void) const { return new octave_matrix (*this); }
  octave_base_value *empty_clone (void) const { return new octave_matrix (); }

  octave_base_value *try_narrowing_conversion (void);

  idx_vector index_vector (void) const { return idx_vector (matrix); }

  bool is_real_matrix (void) const { return true; }

  bool is_real_type (void) const { return true; }

  bool is_double_type (void) const { return true; }

  bool valid_as_scalar_index (void) const;

  int8NDArray
  int8_array_value (void) const { return int8NDArray (matrix); }

  int16NDArray
  int16_array_value (void) const { return int16NDArray (matrix); }

  int32NDArray
  int32_array_value (void) const { return int32NDArray (matrix); }

  int64NDArray
  int64_array_value (void) const { return int64NDArray (matrix); }

  uint8NDArray
  uint8_array_value (void) const { return uint8NDArray (matrix); }

  uint16NDArray
  uint16_array_value (void) const { return uint16NDArray (matrix); }

  uint32NDArray
  uint32_array_value (void) const { return uint32NDArray (matrix); }

  uint64NDArray
  uint64_array_value (void) const { return uint64NDArray (matrix); }

  double double_value (bool = false) const;

  double scalar_value (bool frc_str_conv = false) const
    { return double_value (frc_str_conv); }

  Matrix matrix_value (bool = false) const;

  Complex complex_value (bool = false) const;

  ComplexMatrix complex_matrix_value (bool = false) const;

  ComplexNDArray complex_array_value (bool = false) const;
   
  charNDArray char_array_value (bool = false) const;
  
  NDArray array_value (bool = false) const { return matrix; }

  SparseMatrix sparse_matrix_value (bool = false) const;

  SparseComplexMatrix sparse_complex_matrix_value (bool = false) const;

  streamoff_array streamoff_array_value (void) const;

  void increment (void) { matrix += 1.0; }

  void decrement (void) { matrix -= 1.0; }

  octave_value convert_to_str_internal (bool pad, bool force, char type) const;

  void print_raw (std::ostream& os, bool pr_as_read_syntax = false) const;

  bool save_ascii (std::ostream& os, bool& infnan_warned,
		 bool strip_nan_and_inf);

  bool load_ascii (std::istream& is);

  bool save_binary (std::ostream& os, bool& save_as_floats);

  bool load_binary (std::istream& is, bool swap, 
		    oct_mach_info::float_format fmt);

#if defined (HAVE_HDF5)
  bool save_hdf5 (hid_t loc_id, const char *name, bool save_as_floats);

  bool load_hdf5 (hid_t loc_id, const char *name, bool have_h5giterate_bug);
#endif

  int write (octave_stream& os, int block_size,
	     oct_data_conv::data_type output_type, int skip,
	     oct_mach_info::float_format flt_fmt) const
    { return os.write (matrix, block_size, output_type, skip, flt_fmt); }

private:

  DECLARE_OCTAVE_ALLOCATOR

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
