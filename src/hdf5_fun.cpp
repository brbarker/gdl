/***************************************************************************
                          hdf5_fun.cpp  -  HDF5 GDL library function
                             -------------------
    begin                : Aug 02 2004
    copyright            : (C) 2004 by Peter Messmer
    email                : messmer@users.sourceforge.net
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// the following stuff needs some cleanup in order to make it nicely fit
// into the distribution.

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
// default: assume we have HDF5
#define USE_HDF5 1
#endif

#ifdef USE_HDF5

#include "includefirst.hpp"

#include "hdf5_fun.hpp"

#if defined(H5_USE_16_API) && defined(H5_NO_DEPRECATED_SYMBOLS)
#error "Can't choose old API versions when deprecated APIs are disabled"
#endif /* defined(H5_USE_16_API) && defined(H5_NO_DEPRECATED_SYMBOLS) */

namespace lib {

  using namespace std;

  // SA: error-handling-realted helper routines / classes
  // --------------------------------------------------------------------

  // helper routine for hdf5_error_message()
  herr_t hdf5_error_message_helper(int n, H5E_error_t *err_desc, void *msg)
  {
    // getting something better than "Inappropriate type" message
#if (H5_VERS_MAJOR < 1) || ((H5_VERS_MAJOR == 1) && (H5_VERS_MINOR <= 6))
    if (err_desc->min_num == H5E_BADTYPE)
      *static_cast<string*>(msg) = H5Eget_major(err_desc->maj_num);
    else
      *static_cast<string*>(msg) = H5Eget_minor(err_desc->min_num);
#else
    char* tmp;
    if (err_desc->min_num == H5E_BADTYPE)
      tmp = H5Eget_major(err_desc->maj_num);
    else
      tmp = H5Eget_minor(err_desc->min_num);
    *static_cast<string*>(msg) = tmp;
    free(tmp);
#endif
    return 0;
  }

  // returns a meaningful message describing last HDF5 error
  // usual usege:
  //   if (H5X_xxx(...) < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  string hdf5_error_message(string &msg)
  {
    H5Ewalk(H5E_WALK_UPWARD, hdf5_error_message_helper, &msg);
    return msg;
  }

  // auto_ptr-like class for guarding HDF5 spaces
  // usage:
  //   hid_t h5s_id = H5Dget_space(h5d_id);
  //   if (h5s_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  //   hdf5_space_guard h5s_id_guard = hdf5_space_guard(h5s_id);
  class hdf5_space_guard
  {
    hid_t space;
  public:
    hdf5_space_guard(hid_t space_) { space = space_; }
    ~hdf5_space_guard() { H5Sclose(space); }
  };

  // auto_ptr-like class for guarding HDF5 types
  // usage:
  //   hid_t datatype = H5Dget_type(h5d_id);
  //   if (datatype < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  //   hdf5_type_guard datatype_guard = hdf5_type_guard(datatype);
  class hdf5_type_guard
  {
    hid_t type;
  public:
    hdf5_type_guard(hid_t type_) { type = type_; }
    ~hdf5_type_guard() { H5Tclose(type); }
  };

  // --------------------------------------------------------------------

  DLong mapH5DatatypesToGDL(hid_t h5type){
    //must be in order, from most complicated to simplest, string at end

    //not until LDOUBLE Is handled everywhere!!
    //    if (H5Tequal(h5type , H5T_NATIVE_LDOUBLE )) return GDL_LDOUBLE;

    if (H5Tequal(h5type , H5T_NATIVE_DOUBLE )) return GDL_DOUBLE;
    if (H5Tequal(h5type , H5T_NATIVE_FLOAT )) return GDL_FLOAT;

    if (H5Tequal(h5type , H5T_NATIVE_ULLONG )) return GDL_ULONG64;
    if (H5Tequal(h5type , H5T_ALPHA_U64 )) return GDL_ULONG64;
    if (H5Tequal(h5type , H5T_INTEL_U64 )) return GDL_ULONG64;
    if (H5Tequal(h5type , H5T_MIPS_U64 )) return GDL_ULONG64;
    if (H5Tequal(h5type , H5T_NATIVE_UINT64 )) return GDL_ULONG64;
    if (H5Tequal(h5type , H5T_NATIVE_UINT_FAST64 )) return GDL_ULONG64;
    if (H5Tequal(h5type , H5T_NATIVE_UINT_LEAST64 )) return GDL_ULONG64;
    if (H5Tequal(h5type , H5T_STD_U64BE )) return GDL_ULONG64;
    if (H5Tequal(h5type , H5T_STD_U64LE )) return GDL_ULONG64;


    if (H5Tequal(h5type , H5T_NATIVE_LLONG )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_IEEE_F64BE )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_IEEE_F64LE )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_INTEL_B64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_INTEL_F64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_INTEL_I64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_MIPS_B64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_MIPS_F64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_MIPS_I64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_NATIVE_B64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_NATIVE_INT64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_NATIVE_INT_FAST64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_NATIVE_INT_LEAST64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_STD_B64BE )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_STD_B64LE )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_STD_I64BE )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_STD_I64LE )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_UNIX_D64BE )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_UNIX_D64LE )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_ALPHA_B64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_ALPHA_F64 )) return GDL_LONG64;
    if (H5Tequal(h5type , H5T_ALPHA_I64 )) return GDL_LONG64;


    if (H5Tequal(h5type , H5T_NATIVE_ULONG )) return GDL_ULONG;
    if (H5Tequal(h5type , H5T_ALPHA_U32 )) return GDL_ULONG;
    if (H5Tequal(h5type , H5T_INTEL_U32 )) return GDL_ULONG;
    if (H5Tequal(h5type , H5T_MIPS_U32 )) return GDL_ULONG;
    if (H5Tequal(h5type , H5T_NATIVE_UINT32 )) return GDL_ULONG;
    if (H5Tequal(h5type , H5T_NATIVE_UINT_FAST32 )) return GDL_ULONG;
    if (H5Tequal(h5type , H5T_NATIVE_UINT_LEAST32 )) return GDL_ULONG;
    if (H5Tequal(h5type , H5T_STD_U32BE )) return GDL_ULONG;
    if (H5Tequal(h5type , H5T_STD_U32LE )) return GDL_ULONG;

    if (H5Tequal(h5type , H5T_NATIVE_HBOOL )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_NATIVE_LONG )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_ALPHA_B32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_ALPHA_F32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_ALPHA_I32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_IEEE_F32BE )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_IEEE_F32LE )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_INTEL_B32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_INTEL_F32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_INTEL_I32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_MIPS_B32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_MIPS_F32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_MIPS_I32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_NATIVE_B32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_NATIVE_INT32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_NATIVE_INT_FAST32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_NATIVE_INT_LEAST32 )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_STD_B32BE )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_STD_B32LE )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_STD_I32BE )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_STD_I32LE )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_UNIX_D32BE )) return GDL_LONG;
    if (H5Tequal(h5type , H5T_UNIX_D32LE )) return GDL_LONG;


    if (H5Tequal(h5type , H5T_NATIVE_UINT )) return GDL_UINT;
    if (H5Tequal(h5type , H5T_NATIVE_UINT16 )) return GDL_UINT;
    if (H5Tequal(h5type , H5T_NATIVE_UINT_FAST16 )) return GDL_UINT;
    if (H5Tequal(h5type , H5T_NATIVE_UINT_LEAST16 )) return GDL_UINT;
    if (H5Tequal(h5type , H5T_STD_U16BE )) return GDL_UINT;
    if (H5Tequal(h5type , H5T_STD_U16LE )) return GDL_UINT;
    if (H5Tequal(h5type , H5T_ALPHA_U16 )) return GDL_UINT;
    if (H5Tequal(h5type , H5T_INTEL_U16 )) return GDL_UINT;
    if (H5Tequal(h5type , H5T_MIPS_U16 )) return GDL_UINT;


    if (H5Tequal(h5type , H5T_NATIVE_INT )) return GDL_INT;
    if (H5Tequal(h5type , H5T_NATIVE_INT16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_NATIVE_INT_FAST16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_NATIVE_INT_LEAST16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_STD_B16BE )) return GDL_INT;
    if (H5Tequal(h5type , H5T_STD_B16LE )) return GDL_INT;
    if (H5Tequal(h5type , H5T_STD_I16BE )) return GDL_INT;
    if (H5Tequal(h5type , H5T_STD_I16LE )) return GDL_INT;
    if (H5Tequal(h5type , H5T_ALPHA_B16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_ALPHA_I16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_INTEL_B16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_INTEL_I16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_MIPS_B16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_MIPS_I16 )) return GDL_INT;
    if (H5Tequal(h5type , H5T_NATIVE_B16 )) return GDL_INT;

    if (H5Tequal(h5type , H5T_ALPHA_U8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_MIPS_U8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_INTEL_U8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_NATIVE_UINT8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_NATIVE_UINT_FAST8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_NATIVE_UINT_LEAST8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_STD_U8BE )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_STD_U8LE )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_NATIVE_USHORT )) return GDL_BYTE;

    if (H5Tequal(h5type , H5T_NATIVE_INT8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_ALPHA_B8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_ALPHA_I8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_INTEL_B8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_INTEL_I8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_MIPS_I8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_NATIVE_B8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_NATIVE_INT_FAST8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_NATIVE_INT_LEAST8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_NATIVE_SHORT )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_MIPS_B8 )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_STD_B8BE )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_STD_B8LE )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_STD_I8BE )) return GDL_BYTE;
    if (H5Tequal(h5type , H5T_STD_I8LE )) return GDL_BYTE;

    if (H5Tequal(h5type , H5T_C_S1 )) return GDL_STRING;
    if (H5Tequal(h5type , H5T_FORTRAN_S1 )) return GDL_STRING;
    if (H5Tequal(h5type , H5T_STRING )) return GDL_STRING;

    if (H5Tequal(h5type , H5T_NATIVE_CHAR )) return GDL_STRING;
    if (H5Tequal(h5type , H5T_NATIVE_SCHAR )) return GDL_STRING;
    if (H5Tequal(h5type , H5T_NATIVE_UCHAR )) return GDL_STRING;
    return GDL_UNDEF;
  }

  BaseGDL* h5_get_libversion_fun( EnvT* e)
  {
    unsigned int majnum, minnum, relnum;
    if (H5get_libversion(&majnum, &minnum, &relnum) < 0)
      { string msg; e->Throw(hdf5_error_message(msg)); }
    return new DStringGDL(i2s(majnum) + "." + i2s(minnum) + "." + i2s(relnum));
  }

  /* AC 2019 Jan : managing a major change since Release 1.10.0. !!

https://support.hdfgroup.org/HDF5/doc/ADGuide/Changes.html

This section lists interface-level changes and other user-visible
changes in behavior in the transition from HDF5 Release 1.8.16 to
Release 1.10.0.  Changed Type

hid_t
    Changed from a 32-bit to a 64-bit value.
  */

  hid_t hdf5_input_conversion( EnvT* e, int position)
  {

    hid_t hdf5_id;

#if (H5_VERS_MAJOR>1)||((H5_VERS_MAJOR==1)&&(H5_VERS_MINOR>=10))
    e->AssureLongScalarPar(position, (DLong64&)hdf5_id);
#else
    e->AssureLongScalarPar(position, hdf5_id);
#endif
    return  hdf5_id;
  }

  BaseGDL* hdf5_output_conversion( hid_t h5type)
  {

#if (H5_VERS_MAJOR>1)||((H5_VERS_MAJOR==1)&&(H5_VERS_MINOR>=10))
    return new DLong64GDL(h5type);
#else
    return new DLongGDL(h5type);
#endif

  }

  BaseGDL* h5f_is_hdf5_fun( EnvT* e)
  {
    DString h5fFilename;
    e->AssureScalarPar<DStringGDL>( 0, h5fFilename);
    WordExp( h5fFilename);
    htri_t code;
    code = H5Fis_hdf5(h5fFilename.c_str());
    if (code <= 0) return new DLongGDL(0); else  return new DLongGDL(1);
  }


  BaseGDL* h5f_create_fun( EnvT* e)
  {

    DString h5fFilename;
    e->AssureScalarPar<DStringGDL>( 0, h5fFilename);
    WordExp( h5fFilename);

    hid_t h5f_id;
    h5f_id = H5Fcreate( h5fFilename.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);

    if (h5f_id < 0)
      {
        string msg;
        e->Throw(hdf5_error_message(msg));
      }

    return hdf5_output_conversion( h5f_id );
  }

  BaseGDL* h5f_open_fun( EnvT* e)
  {

    DString h5fFilename;
    e->AssureScalarPar<DStringGDL>( 0, h5fFilename);
    WordExp( h5fFilename);

    hid_t h5f_id;
    h5f_id = H5Fopen(h5fFilename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

    if (h5f_id < 0)
      {
        string msg;
        e->Throw(hdf5_error_message(msg));
      }

    return hdf5_output_conversion( h5f_id );
  }

  BaseGDL* h5g_open_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(2);

    hid_t h5f_id = hdf5_input_conversion(e, 0);

    DString h5gGroupname;
    e->AssureScalarPar<DStringGDL>( 1, h5gGroupname);

    hid_t h5g_id;
    h5g_id = H5Gopen(h5f_id, h5gGroupname.c_str());
    if (h5g_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return hdf5_output_conversion( h5g_id );

  }

  BaseGDL* h5d_open_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(2);

    hid_t h5f_id = hdf5_input_conversion(e,0);

    DString h5dDatasetname;
    e->AssureScalarPar<DStringGDL>( 1, h5dDatasetname);

    hid_t h5d_id= H5Dopen((long)h5f_id, h5dDatasetname.c_str());

    if (h5d_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return hdf5_output_conversion( h5d_id );

  }

  BaseGDL* h5a_open_idx_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(2);

    hid_t h5f_id = hdf5_input_conversion(e,0);
    hid_t attr_idx = hdf5_input_conversion(e,1);

    hid_t h5a_id = H5Aopen_idx(h5f_id, attr_idx);
    if (h5a_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return hdf5_output_conversion( h5a_id );
  }


  BaseGDL* h5a_get_name_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5a_id = hdf5_input_conversion(e,0);

    // querying for the length of the name
    char tmp;
    ssize_t len = H5Aget_name(h5a_id, 1, &tmp);
    if (len < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    // acquiring the name
    len++;
    char* name = static_cast<char*>(malloc(len * sizeof(char)));
    if (name == NULL) e->Throw("Failed to allocate memory!");
    if (H5Aget_name(h5a_id, len, name) < 0)
      {
        free(name);
        { string msg; e->Throw(hdf5_error_message(msg)); }
        return NULL;
      }
    DStringGDL* ret = new DStringGDL(name);
    free(name);
    return ret;
  }


  BaseGDL* h5a_get_type_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5a_id = hdf5_input_conversion(e,0);

    hid_t h5a_type_id;
    h5a_type_id = H5Aget_type( h5a_id );
    if (h5a_type_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return hdf5_output_conversion( h5a_type_id);
  }


  BaseGDL* h5a_open_name_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(2);

    hid_t h5f_id = hdf5_input_conversion(e,0);

    DString h5aAttrname;
    e->AssureScalarPar<DStringGDL>( 1, h5aAttrname);

    hid_t h5a_id = H5Aopen_name(h5f_id, h5aAttrname.c_str());
    if (h5a_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return hdf5_output_conversion( h5a_id);
  }


  BaseGDL* h5d_get_space_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5d_id = hdf5_input_conversion(e,0);

    hid_t h5d_space_id = H5Dget_space( h5d_id );
    if (h5d_space_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return hdf5_output_conversion( h5d_space_id );
  }

  BaseGDL* h5a_get_space_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5a_id=hdf5_input_conversion(e,0);

    hid_t h5a_space_id = H5Aget_space( h5a_id );
    if (h5a_space_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return hdf5_output_conversion( h5a_space_id );

  }

  BaseGDL* h5a_get_num_attrs_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t loc_id=hdf5_input_conversion(e,0);

    int num = H5Aget_num_attrs( loc_id );
    if (num < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    // following the doc., should return a "int"
    return new DLongGDL( num );
  }


  BaseGDL* h5d_get_type_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5d_id=hdf5_input_conversion(e,0);

    hid_t h5d_type_id = H5Dget_type( h5d_id );
    if (h5d_type_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return hdf5_output_conversion( h5d_type_id );
  }


  BaseGDL* h5t_get_size_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5t_id=hdf5_input_conversion(e,0);

    // following the doc., should return a "size_t"
    size_t size = H5Tget_size( h5t_id );
    if (size == 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return new DLongGDL( size );

  }


  BaseGDL* h5s_get_simple_extent_ndims_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5s_id=hdf5_input_conversion(e,0);

    int rank = H5Sget_simple_extent_ndims(h5s_id);
    if (rank < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    return new DLongGDL(rank);
  }


  BaseGDL* h5s_get_simple_extent_dims_fun( EnvT* e)
  {
    SizeT nParam=e->NParam(1);
    hsize_t dims_out[MAXRANK];

    hid_t h5s_id=hdf5_input_conversion(e,0);

    int rank = H5Sget_simple_extent_ndims(h5s_id);
    if (rank < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    if (H5Sget_simple_extent_dims(h5s_id, dims_out, NULL) < 0)
      { string msg; e->Throw(hdf5_error_message(msg)); }

    dimension dim(rank);
    DLongGDL* d = new DLongGDL(dim);

    for(int i=0; i<rank; i++)
      (*d)[i] = dims_out[rank - 1 - i];
    return d;
  }


  BaseGDL* h5a_read_fun( EnvT* e)
  {

     /* Jun 2021, Oliver Gressel <ogressel@gmail.com>
        - add support for attributes of type 'H5T_ARRAY'
     */

    bool debug = false;

    SizeT nParam=e->NParam(1);
    hsize_t dims_out[MAXRANK];
    hsize_t elem_dims[MAXRANK];

    hid_t h5a_id = hdf5_input_conversion(e,0);

    hid_t h5s_id = H5Aget_space(h5a_id);

    if (h5s_id < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
    hdf5_space_guard h5s_id_guard = hdf5_space_guard(h5s_id);

    hid_t datatype = H5Aget_type(h5a_id);
    if (datatype < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
    hdf5_type_guard datatype_guard = hdf5_type_guard(datatype);

    // for array datatypes, determine the rank and dimension of the element
    int elem_rank=0;
    hid_t elem_dtype;

    if (H5Tget_class(datatype)==H5T_ARRAY ) {

       if ((elem_rank=H5Tget_array_ndims(datatype)) <0)
          { string msg; e->Throw(hdf5_error_message(msg)); }
       if (debug) cout << "array datatype of rank " << elem_rank << endl;

       if (H5Tget_array_dims2(datatype, elem_dims) <0)
          { string msg; e->Throw(hdf5_error_message(msg)); }

       if (debug && elem_rank>0) {
          cout << "dimensions are: ";
          for(int i=0; i<elem_rank; i++) cout << elem_dims[i] << ",";
          cout << endl;
       }

       elem_dtype = H5Tget_super(datatype);
       if (elem_dtype < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    } else {

       elem_dtype = H5Tcopy(datatype);
       if (elem_dtype < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    }

    hdf5_type_guard elem_dtype_guard = hdf5_type_guard(elem_dtype);

    // determine the rank and dimension of the dataset
    int rank = H5Sget_simple_extent_ndims(h5s_id);
    if (rank < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    if (debug) cout << "attribute data rank is " << rank << endl;

    if (H5Sget_simple_extent_dims(h5s_id, dims_out, NULL) < 0)
      { string msg; e->Throw(hdf5_error_message(msg)); }

    if (debug && rank>0) {
       cout << "dimensions are: ";
       for(int i=0; i<rank; i++) cout << dims_out[i] << ",";
       cout << endl;
    }

    // need to reverse indices for column major format
    SizeT count_s[MAXRANK];
    for(int i=0; i<elem_rank; i++)
      count_s[i] = (SizeT)elem_dims[elem_rank - 1  - i ];

    for(int i=elem_rank; i<elem_rank+rank; i++)
      count_s[i] = (SizeT)dims_out[elem_rank+rank - 1  - i ];

    // create the IDL datatypes
    dimension dim(count_s, rank+elem_rank);

    BaseGDL *res;
    if (debug) cout << "datatype : " << elem_dtype << endl;
    DLong ourType = mapH5DatatypesToGDL(elem_dtype);
    hsize_t type;

    if (ourType == GDL_BYTE) {
      res = new DByteGDL(dim);
      type = H5T_NATIVE_UINT8;
    } else if (ourType == GDL_INT) {
      res = new DIntGDL(dim);
      type = H5T_NATIVE_INT16;
    } else if (ourType == GDL_UINT) {
      res = new DUIntGDL(dim);
      type = H5T_NATIVE_UINT16;
    } else if (ourType == GDL_LONG) {
      res = new DLongGDL(dim);
      type = H5T_NATIVE_INT32;
    } else if (ourType == GDL_ULONG) {
      res = new DULongGDL(dim);
      type = H5T_NATIVE_UINT32;
    } else if (ourType == GDL_LONG64) {
      res = new DLong64GDL(dim);
      type = H5T_NATIVE_INT64;
    } else if (ourType == GDL_LONG64) {
      res = new DULong64GDL(dim);
      type = H5T_NATIVE_UINT64;
    } else if (ourType == GDL_FLOAT) {
      res = new DFloatGDL(dim);
      type = H5T_NATIVE_FLOAT;
    } else if (ourType == GDL_DOUBLE) {
      res = new DDoubleGDL(dim);
      type = H5T_NATIVE_DOUBLE;
    } else if (ourType == GDL_STRING) {

      ///******* BELOW CODE IS NOT TESTED! ********///

      // a bit special, lets follow the example on h5 site:
      res = new DStringGDL(dim);
      type = H5T_C_S1;
      hid_t filetype = H5Dget_type(h5a_id);
      SizeT sdim = H5Tget_size(filetype);
      sdim++; /* Make room for null terminator */
      char **rdata;
      /*
       * Allocate array of pointers to rows.
       */
      rdata = (char **) malloc(count_s[0] * sizeof (char *));
      /*
       * Allocate space for integer data.
       */
      rdata[0] = (char *) malloc(count_s[0] * sdim * sizeof (char));
      /*
       * Set the rest of the pointers to rows to the correct addresses.
       */
      for (int i = 1; i < count_s[0]; i++)
        rdata[i] = rdata[0] + i * sdim;
      /*
       * Create the memory datatype.
       */
      hid_t memtype = H5Tcopy(H5T_C_S1);
      hid_t status = H5Tset_size(memtype, sdim);

      if (H5Aread(h5a_id, type, rdata[0]) < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
      for (int i = 0; i < count_s[0]; i++)
        (*(static_cast<DStringGDL*> (res)))[i] = rdata[i];
      free (rdata); //but not rdata[0]
      status = H5Tclose (filetype);
      status = H5Tclose (memtype);
      return res;
    } else {
      e->Throw("Unsupported data format" + i2s(elem_dtype));
    }

    if (elem_rank>0) type = H5Tarray_create2( type, elem_rank, elem_dims );

    if (H5Aread(h5a_id, type, res->DataAddr()) < 0)
      { string msg; e->Throw(hdf5_error_message(msg)); }

    if (elem_rank>0) H5Tclose(type);

    return res;

    return new DIntGDL(-1);
  }


  /**
   * h5d_read_fun
   * CAUTION: compatibility only fractional
   * - Does not allow to select hyperslabs
   */
  BaseGDL* h5d_read_fun(EnvT* e) {

    /* Jul 2021, Oliver Gressel <ogressel@gmail.com>
       - add support for datasets of type 'H5T_ARRAY'
    */

    bool debug = false;

    SizeT nParam = e->NParam(1);
    hsize_t dims_out[MAXRANK];
    hsize_t elem_dims[MAXRANK];

    hid_t h5d_id = hdf5_input_conversion(e,0);

    hid_t h5s_id = H5Dget_space(h5d_id);
    if (h5s_id < 0) {
      string msg;
      e->Throw(hdf5_error_message(msg));
    }
    hdf5_space_guard h5s_id_guard = hdf5_space_guard(h5s_id);

    hid_t datatype = H5Dget_type(h5d_id);
    if (datatype < 0) {
      string msg;
      e->Throw(hdf5_error_message(msg));
    }

    if (debug) cout << "here 1" <<endl;

    hdf5_type_guard datatype_guard = hdf5_type_guard(datatype);

    // for array datatypes, determine the rank and dimension of the element
    int elem_rank=0;
    hid_t elem_dtype;

    if (H5Tget_class(datatype)==H5T_ARRAY ) {

       if ((elem_rank=H5Tget_array_ndims(datatype)) <0)
          { string msg; e->Throw(hdf5_error_message(msg)); }
       if (debug) cout << "array datatype of rank " << elem_rank << endl;

       if (H5Tget_array_dims2(datatype, elem_dims) <0)
          { string msg; e->Throw(hdf5_error_message(msg)); }

       if (debug && elem_rank>0) {
          cout << "dimensions are: ";
          for(int i=0; i<elem_rank; i++) cout << elem_dims[i] << ",";
          cout << endl;
       }

       elem_dtype = H5Tget_super(datatype);
       if (elem_dtype < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    } else {

       elem_dtype = H5Tcopy(datatype);
       if (elem_dtype < 0) { string msg; e->Throw(hdf5_error_message(msg)); }

    }

    hdf5_type_guard elem_dtype_guard = hdf5_type_guard(elem_dtype);

    // determine the rank and dimension of the dataset
    int rank = H5Sget_simple_extent_ndims(h5s_id);
    if (rank < 0) {
      string msg;
      e->Throw(hdf5_error_message(msg));
    }
    if (debug) cout << "data rank is " << rank << endl;

    if (H5Sget_simple_extent_dims(h5s_id, dims_out, NULL) < 0) {
      string msg;
      e->Throw(hdf5_error_message(msg));
    }

    if (debug && rank>0) {
       cout << "dimensions are: ";
       for(int i=0; i<rank; i++) cout << dims_out[i] << ",";
       cout << endl;
    }

    // define hyperslab in dataset
#if (H5_VERS_MAJOR < 1) || ((H5_VERS_MAJOR == 1) && (H5_VERS_MINOR < 6)) ||  ((H5_VERS_MAJOR == 1) && (H5_VERS_MINOR == 6) && (H5_VERS_RELEASE <= 3))
    hssize_t offset[MAXRANK];
#else
    hsize_t offset[MAXRANK];
#endif
    hsize_t count[MAXRANK];

    if (rank>0) {

       for (int i = 0; i < rank; i++) offset[i] = 0;
       for (int i = 0; i < rank; i++) count[i] = dims_out[i];

       if (H5Sselect_hyperslab(h5s_id, H5S_SELECT_SET,
                               offset, NULL, count, NULL) < 0) {
          string msg;
          e->Throw(hdf5_error_message(msg));
       }
    }
    if (debug) cout << "here 2" <<endl;

    // define memory dataspace
    hid_t memspace = H5Screate_simple(rank, count, NULL);
    if (memspace < 0) {
       string msg;
       e->Throw(hdf5_error_message(msg));
    }
    hdf5_space_guard memspace_guard = hdf5_space_guard(memspace);

    // define memory hyperslab
#if (H5_VERS_MAJOR < 1) || ((H5_VERS_MAJOR == 1) && (H5_VERS_MINOR < 6)) ||  ((H5_VERS_MAJOR == 1) && (H5_VERS_MINOR == 6) && (H5_VERS_RELEASE <= 3))
    hssize_t offset_out[MAXRANK];
#else
    hsize_t offset_out[MAXRANK];
#endif
    hsize_t count_out[MAXRANK];

    if (rank>0) {

       for (int i = 0; i < rank; i++) offset_out[i] = 0;
       for (int i = 0; i < rank; i++) count_out[i] = dims_out[i];

       if (H5Sselect_hyperslab(memspace, H5S_SELECT_SET,
                               offset_out, NULL, count_out, NULL) < 0) {
          string msg;
          e->Throw(hdf5_error_message(msg));
       }
    }
    if (debug) cout << "here 3" <<endl;

    SizeT count_s[MAXRANK];
    SizeT rank_s;

    rank_s = (SizeT) (elem_rank + rank);

    // need to reverse indices for column major format
    for(int i=0; i<elem_rank; i++)
      count_s[i] = (SizeT)elem_dims[elem_rank - 1  - i ];

    for(int i=elem_rank; i<elem_rank+rank; i++)
      count_s[i] = (SizeT)dims_out[elem_rank+rank - 1  - i ];

    // create the IDL datatypes
    dimension dim(count_s, rank_s);

    BaseGDL *res;

    if (debug) cout << "datatype : " << elem_dtype << endl;

    DLong ourType = mapH5DatatypesToGDL(elem_dtype);
    hsize_t type;

    if (debug)  cout << "ourType : " << ourType  << endl;
    if (debug) cout << "GDL_STRING : " << GDL_STRING << endl;

    if (ourType == GDL_BYTE) {
      res = new DByteGDL(dim);
      type = H5T_NATIVE_UINT8;
    } else if (ourType == GDL_INT) {
      res = new DIntGDL(dim);
      type = H5T_NATIVE_INT16;
    } else if (ourType == GDL_UINT) {
      res = new DUIntGDL(dim);
      type = H5T_NATIVE_UINT16;
    } else if (ourType == GDL_LONG) {
      res = new DLongGDL(dim);
      type = H5T_NATIVE_INT32;
    } else if (ourType == GDL_ULONG) {
      res = new DULongGDL(dim);
      type = H5T_NATIVE_UINT32;
    } else if (ourType == GDL_LONG64) {
      res = new DLong64GDL(dim);
      type = H5T_NATIVE_INT64;
    } else if (ourType == GDL_LONG64) {
      res = new DULong64GDL(dim);
      type = H5T_NATIVE_UINT64;
    } else if (ourType == GDL_FLOAT) {
      res = new DFloatGDL(dim);
      type = H5T_NATIVE_FLOAT;
    } else if (ourType == GDL_DOUBLE) {
      res = new DDoubleGDL(dim);
      type = H5T_NATIVE_DOUBLE;
    } else if (ourType == GDL_STRING) {

      if (debug) cout << "here 4a" <<endl;

      // a bit special, lets follow the example on h5 site:
      res = new DStringGDL(dim);
      type = H5T_C_S1;
      hid_t filetype = H5Dget_type(h5d_id);
      SizeT sdim = H5Tget_size(filetype);
      sdim++; /* Make room for null terminator */
      char **rdata;
      /*
       * Allocate array of pointers to rows.
       */
      rdata = (char **) malloc(count_s[0] * sizeof (char *));
      /*
       * Allocate space for integer data.
       */
      rdata[0] = (char *) malloc(count_s[0] * sdim * sizeof (char));
      /*
       * Set the rest of the pointers to rows to the correct addresses.
       */
      for (int i = 1; i < count_s[0]; i++)
        rdata[i] = rdata[0] + i * sdim;
      /*
       * Create the memory datatype.
       */
      hid_t memtype = H5Tcopy(H5T_C_S1);
      hid_t status = H5Tset_size(memtype, sdim);

      if (debug) cout << "here 4b" <<endl;

      status = H5Dread(h5d_id, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata[0]);

      if (debug) cout << "here 4c" <<endl;

      if (status < 0) {
        string msg;
        e->Throw(hdf5_error_message(msg));
      }
      if (debug) cout << "here 4d" <<endl;

      for (int i = 0; i < count_s[0]; i++)
        (*(static_cast<DStringGDL*> (res)))[i] = rdata[i];
      free (rdata); //but not rdata[0]
      status = H5Tclose (filetype);
      status = H5Tclose (memtype);
      return res;
    } else {
      e->Throw("Unsupported data format" + i2s(elem_dtype));
    }

    if (debug) cout << "here 5" <<endl;

    if (elem_rank>0) type = H5Tarray_create2( type, elem_rank, elem_dims );

    if (H5Dread(h5d_id, type, memspace, h5s_id,
                H5P_DEFAULT, res->DataAddr()) < 0) {
      string msg;
      e->Throw(hdf5_error_message(msg));
    }

    if (elem_rank>0) H5Tclose(type);

    return res;
  }


  void h5s_close_pro( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5s_id = hdf5_input_conversion(e,0);

    if (H5Sclose(h5s_id) < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  }


  void h5d_close_pro( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5d_id = hdf5_input_conversion(e,0);

    if (H5Dclose(h5d_id) < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  }


  void h5f_close_pro( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5f_id = hdf5_input_conversion(e,0);

    if (H5Fclose(h5f_id) < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  }


  void h5t_close_pro( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5t_id = hdf5_input_conversion(e,0);

    if (H5Tclose(h5t_id) < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  }


  void h5g_close_pro( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5g_id = hdf5_input_conversion(e,0);

    if (H5Gclose(h5g_id) < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  }


  void h5a_close_pro( EnvT* e)
  {
    SizeT nParam=e->NParam(1);

    hid_t h5a_id = hdf5_input_conversion(e,0);

    if (H5Aclose(h5a_id) < 0) { string msg; e->Throw(hdf5_error_message(msg)); }
  }

} // namespace

#endif
