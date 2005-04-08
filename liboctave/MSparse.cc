/*

Copyright (C) 2004 David Bateman
Copyright (C) 1998-2004 Andy Adler

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "quit.h"
#include "lo-error.h"
#include "MArray2.h"
#include "Array-util.h"

#include "MSparse.h"
#include "MSparse-defs.h"

// sparse array with math ops.

// Element by element MSparse by MSparse ops.

template <class T>
MSparse<T>&
operator += (MSparse<T>& a, const MSparse<T>& b)
{
    MSparse<T> r;

    octave_idx_type a_nr = a.rows ();
    octave_idx_type a_nc = a.cols ();

    octave_idx_type b_nr = b.rows ();
    octave_idx_type b_nc = b.cols ();

    if (a_nr != b_nr || a_nc != b_nc)
      gripe_nonconformant ("operator +=" , a_nr, a_nc, b_nr, b_nc);
    else
      {
        r = MSparse<T> (a_nr, a_nc, (a.nnz () + b.nnz ()));
       
        octave_idx_type jx = 0;
        for (octave_idx_type i = 0 ; i < a_nc ; i++)
          {
            octave_idx_type  ja = a.cidx(i);
            octave_idx_type  ja_max = a.cidx(i+1);
            bool ja_lt_max= ja < ja_max;
           
            octave_idx_type  jb = b.cidx(i);
            octave_idx_type  jb_max = b.cidx(i+1);
            bool jb_lt_max = jb < jb_max;
           
            while (ja_lt_max || jb_lt_max )
              {
                OCTAVE_QUIT;
                if ((! jb_lt_max) ||
                      (ja_lt_max && (a.ridx(ja) < b.ridx(jb))))
                  {
                    r.ridx(jx) = a.ridx(ja);
                    r.data(jx) = a.data(ja) + 0.;
                    jx++;
                    ja++;
                    ja_lt_max= ja < ja_max;
                  }
                else if (( !ja_lt_max ) ||
                     (jb_lt_max && (b.ridx(jb) < a.ridx(ja)) ) )
                  {
		    r.ridx(jx) = b.ridx(jb);
		    r.data(jx) = 0. + b.data(jb);
		    jx++;
                    jb++;
                    jb_lt_max= jb < jb_max;
                  }
                else
                  {
		     if ((a.data(ja) + b.data(jb)) != 0.)
	               {
                          r.data(jx) = a.data(ja) + b.data(jb);
                          r.ridx(jx) = a.ridx(ja);
                          jx++;
                       }
                     ja++;
                     ja_lt_max= ja < ja_max;
                     jb++;
                     jb_lt_max= jb < jb_max;
                  }
              }
            r.cidx(i+1) = jx;
          }
       
	a = r.maybe_compress ();
      }

    return a;
}

template <class T>
MSparse<T>&
operator -= (MSparse<T>& a, const MSparse<T>& b)
{
    MSparse<T> r;

    octave_idx_type a_nr = a.rows ();
    octave_idx_type a_nc = a.cols ();

    octave_idx_type b_nr = b.rows ();
    octave_idx_type b_nc = b.cols ();

    if (a_nr != b_nr || a_nc != b_nc)
      gripe_nonconformant ("operator -=" , a_nr, a_nc, b_nr, b_nc);
    else
      {
        r = MSparse<T> (a_nr, a_nc, (a.nnz () + b.nnz ()));
       
        octave_idx_type jx = 0;
        for (octave_idx_type i = 0 ; i < a_nc ; i++)
          {
            octave_idx_type  ja = a.cidx(i);
            octave_idx_type  ja_max = a.cidx(i+1);
            bool ja_lt_max= ja < ja_max;
           
            octave_idx_type  jb = b.cidx(i);
            octave_idx_type  jb_max = b.cidx(i+1);
            bool jb_lt_max = jb < jb_max;
           
            while (ja_lt_max || jb_lt_max )
              {
                OCTAVE_QUIT;
                if ((! jb_lt_max) ||
                      (ja_lt_max && (a.ridx(ja) < b.ridx(jb))))
                  {
                    r.ridx(jx) = a.ridx(ja);
                    r.data(jx) = a.data(ja) - 0.;
                    jx++;
                    ja++;
                    ja_lt_max= ja < ja_max;
                  }
                else if (( !ja_lt_max ) ||
                     (jb_lt_max && (b.ridx(jb) < a.ridx(ja)) ) )
                  {
		    r.ridx(jx) = b.ridx(jb);
		    r.data(jx) = 0. - b.data(jb);
		    jx++;
                    jb++;
                    jb_lt_max= jb < jb_max;
                  }
                else
                  {
		     if ((a.data(ja) - b.data(jb)) != 0.)
	               {
                          r.data(jx) = a.data(ja) - b.data(jb);
                          r.ridx(jx) = a.ridx(ja);
                          jx++;
                       }
                     ja++;
                     ja_lt_max= ja < ja_max;
                     jb++;
                     jb_lt_max= jb < jb_max;
                  }
              }
            r.cidx(i+1) = jx;
          }
       
	a = r.maybe_compress ();
      }

    return a;
}

// Element by element MSparse by scalar ops.

#define SPARSE_A2S_OP_1(OP) \
  template <class T> \
  MArray2<T> \
  operator OP (const MSparse<T>& a, const T& s) \
  { \
    octave_idx_type nr = a.rows (); \
    octave_idx_type nc = a.cols (); \
 \
    MArray2<T> r (nr, nc, (0.0 OP s));	\
 \
    for (octave_idx_type j = 0; j < nc; j++) \
      for (octave_idx_type i = a.cidx(j); i < a.cidx(j+1); i++)	\
        r.elem (a.ridx (i), j) = a.data (i) OP s;	\
    return r; \
  }

#define SPARSE_A2S_OP_2(OP) \
  template <class T> \
  MSparse<T> \
  operator OP (const MSparse<T>& a, const T& s) \
  { \
    octave_idx_type nr = a.rows (); \
    octave_idx_type nc = a.cols (); \
    octave_idx_type nz = a.nnz (); \
 \
    MSparse<T> r (nr, nc, nz); \
 \
    for (octave_idx_type i = 0; i < nz; i++) \
      { \
	r.data(i) = a.data(i) OP s; \
	r.ridx(i) = a.ridx(i); \
      } \
    for (octave_idx_type i = 0; i < nc + 1; i++) \
      r.cidx(i) = a.cidx(i); \
    r.maybe_compress (true); \
    return r; \
  }


SPARSE_A2S_OP_1 (+)
SPARSE_A2S_OP_1 (-)
SPARSE_A2S_OP_2 (*)
SPARSE_A2S_OP_2 (/)

// Element by element scalar by MSparse ops.

#define SPARSE_SA2_OP_1(OP) \
  template <class T> \
  MArray2<T> \
  operator OP (const T& s, const MSparse<T>& a) \
  { \
    octave_idx_type nr = a.rows (); \
    octave_idx_type nc = a.cols (); \
 \
    MArray2<T> r (nr, nc, (s OP 0.0));	\
 \
    for (octave_idx_type j = 0; j < nc; j++) \
      for (octave_idx_type i = a.cidx(j); i < a.cidx(j+1); i++)	\
        r.elem (a.ridx (i), j) = s OP a.data (i);	\
    return r; \
  }

#define SPARSE_SA2_OP_2(OP) \
  template <class T> \
  MSparse<T> \
  operator OP (const T& s, const MSparse<T>& a) \
  { \
    octave_idx_type nr = a.rows (); \
    octave_idx_type nc = a.cols (); \
    octave_idx_type nz = a.nnz (); \
 \
    MSparse<T> r (nr, nc, nz); \
 \
    for (octave_idx_type i = 0; i < nz; i++) \
      { \
	r.data(i) = s OP a.data(i); \
	r.ridx(i) = a.ridx(i); \
      } \
    for (octave_idx_type i = 0; i < nc + 1; i++) \
      r.cidx(i) = a.cidx(i); \
    r.maybe_compress (true); \
    return r; \
  }

SPARSE_SA2_OP_1 (+)
SPARSE_SA2_OP_1 (-)
SPARSE_SA2_OP_2 (*)
SPARSE_SA2_OP_2 (/)

// Element by element MSparse by MSparse ops.

#define SPARSE_A2A2_OP(OP) \
  template <class T> \
  MSparse<T> \
  operator OP (const MSparse<T>& a, const MSparse<T>& b) \
  { \
    MSparse<T> r; \
 \
    octave_idx_type a_nr = a.rows (); \
    octave_idx_type a_nc = a.cols (); \
 \
    octave_idx_type b_nr = b.rows (); \
    octave_idx_type b_nc = b.cols (); \
 \
    if (a_nr != b_nr || a_nc != b_nc) \
      gripe_nonconformant ("operator " # OP, a_nr, a_nc, b_nr, b_nc); \
    else \
      { \
        r = MSparse<T> (a_nr, a_nc, (a.nnz () + b.nnz ())); \
        \
        octave_idx_type jx = 0; \
	r.cidx (0) = 0; \
        for (octave_idx_type i = 0 ; i < a_nc ; i++) \
          { \
            octave_idx_type  ja = a.cidx(i); \
            octave_idx_type  ja_max = a.cidx(i+1); \
            bool ja_lt_max= ja < ja_max; \
            \
            octave_idx_type  jb = b.cidx(i); \
            octave_idx_type  jb_max = b.cidx(i+1); \
            bool jb_lt_max = jb < jb_max; \
            \
            while (ja_lt_max || jb_lt_max ) \
              { \
                OCTAVE_QUIT; \
                if ((! jb_lt_max) || \
                      (ja_lt_max && (a.ridx(ja) < b.ridx(jb)))) \
                  { \
                    r.ridx(jx) = a.ridx(ja); \
                    r.data(jx) = a.data(ja) OP 0.; \
                    jx++; \
                    ja++; \
                    ja_lt_max= ja < ja_max; \
                  } \
                else if (( !ja_lt_max ) || \
                     (jb_lt_max && (b.ridx(jb) < a.ridx(ja)) ) ) \
                  { \
		    r.ridx(jx) = b.ridx(jb); \
		    r.data(jx) = 0. OP b.data(jb); \
		    jx++; \
                    jb++; \
                    jb_lt_max= jb < jb_max; \
                  } \
                else \
                  { \
		     if ((a.data(ja) OP b.data(jb)) != 0.) \
	               { \
                          r.data(jx) = a.data(ja) OP b.data(jb); \
                          r.ridx(jx) = a.ridx(ja); \
                          jx++; \
                       } \
                     ja++; \
                     ja_lt_max= ja < ja_max; \
                     jb++; \
                     jb_lt_max= jb < jb_max; \
                  } \
              } \
            r.cidx(i+1) = jx; \
          } \
        \
	r.maybe_compress (); \
      } \
 \
    return r; \
  }

#define SPARSE_A2A2_FCN_1(FCN, OP)	\
  template <class T> \
  MSparse<T> \
  FCN (const MSparse<T>& a, const MSparse<T>& b) \
  { \
    MSparse<T> r; \
 \
    octave_idx_type a_nr = a.rows (); \
    octave_idx_type a_nc = a.cols (); \
 \
    octave_idx_type b_nr = b.rows (); \
    octave_idx_type b_nc = b.cols (); \
 \
    if (a_nr != b_nr || a_nc != b_nc) \
      gripe_nonconformant (#FCN, a_nr, a_nc, b_nr, b_nc); \
    else \
      { \
        r = MSparse<T> (a_nr, a_nc, (a.nnz() > b.nnz() ? a.nnz() : b.nnz())); \
        \
        octave_idx_type jx = 0; \
	r.cidx (0) = 0; \
        for (octave_idx_type i = 0 ; i < a_nc ; i++) \
          { \
            octave_idx_type  ja = a.cidx(i); \
            octave_idx_type  ja_max = a.cidx(i+1); \
            bool ja_lt_max= ja < ja_max; \
            \
            octave_idx_type  jb = b.cidx(i); \
            octave_idx_type  jb_max = b.cidx(i+1); \
            bool jb_lt_max = jb < jb_max; \
            \
            while (ja_lt_max || jb_lt_max ) \
              { \
                OCTAVE_QUIT; \
                if ((! jb_lt_max) || \
                      (ja_lt_max && (a.ridx(ja) < b.ridx(jb)))) \
                  { \
                     ja++; ja_lt_max= ja < ja_max; \
                  } \
                else if (( !ja_lt_max ) || \
                     (jb_lt_max && (b.ridx(jb) < a.ridx(ja)) ) ) \
                  { \
                     jb++; jb_lt_max= jb < jb_max; \
                  } \
                else \
                  { \
		     if ((a.data(ja) OP b.data(jb)) != 0.) \
	               { \
                          r.data(jx) = a.data(ja) OP b.data(jb); \
                          r.ridx(jx) = a.ridx(ja); \
                          jx++; \
                       } \
                     ja++; ja_lt_max= ja < ja_max; \
                     jb++; jb_lt_max= jb < jb_max; \
                  } \
              } \
            r.cidx(i+1) = jx; \
          } \
        \
	r.maybe_compress (); \
      } \
 \
    return r; \
  }

#define SPARSE_A2A2_FCN_2(FCN, OP)	\
  template <class T> \
  MSparse<T> \
  FCN (const MSparse<T>& a, const MSparse<T>& b) \
  { \
    MSparse<T> r; \
    T Zero = T (); \
 \
    octave_idx_type a_nr = a.rows (); \
    octave_idx_type a_nc = a.cols (); \
 \
    octave_idx_type b_nr = b.rows (); \
    octave_idx_type b_nc = b.cols (); \
 \
    if (a_nr != b_nr || a_nc != b_nc) \
      gripe_nonconformant (#FCN, a_nr, a_nc, b_nr, b_nc); \
    else \
      { \
        r = MSparse<T>( a_nr, a_nc, (Zero OP Zero)); \
        \
        for (octave_idx_type i = 0 ; i < a_nc ; i++) \
          { \
            octave_idx_type  ja = a.cidx(i); \
            octave_idx_type  ja_max = a.cidx(i+1); \
            bool ja_lt_max= ja < ja_max; \
            \
            octave_idx_type  jb = b.cidx(i); \
            octave_idx_type  jb_max = b.cidx(i+1); \
            bool jb_lt_max = jb < jb_max; \
            \
            while (ja_lt_max || jb_lt_max ) \
              { \
                OCTAVE_QUIT; \
                if ((! jb_lt_max) || \
                      (ja_lt_max && (a.ridx(ja) < b.ridx(jb)))) \
                  { \
		     r.elem (a.ridx(ja),i) = a.data(ja) OP Zero; \
                     ja++; ja_lt_max= ja < ja_max; \
                  } \
                else if (( !ja_lt_max ) || \
                     (jb_lt_max && (b.ridx(jb) < a.ridx(ja)) ) ) \
                  { \
		     r.elem (b.ridx(jb),i) = Zero OP b.data(jb);	\
                     jb++; jb_lt_max= jb < jb_max; \
                  } \
                else \
                  { \
                     r.elem (a.ridx(ja),i) = a.data(ja) OP b.data(jb); \
                     ja++; ja_lt_max= ja < ja_max; \
                     jb++; jb_lt_max= jb < jb_max; \
                  } \
              } \
          } \
        \
	r.maybe_compress (true); \
      } \
 \
    return r; \
  }

SPARSE_A2A2_OP (+)
SPARSE_A2A2_OP (-)
SPARSE_A2A2_FCN_1 (product,    *)
SPARSE_A2A2_FCN_2 (quotient,   /)

// Unary MSparse ops.

template <class T>
MSparse<T>
operator + (const MSparse<T>& a)
{
  return a;
}

template <class T>
MSparse<T>
operator - (const MSparse<T>& a)
{
  MSparse<T> retval (a);
  octave_idx_type nz = a.nnz ();
  for (octave_idx_type i = 0; i < nz; i++)
    retval.data(i) = - retval.data(i);
  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
