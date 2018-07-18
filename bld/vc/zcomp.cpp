
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vccomp.h"
#include "zlib.h"
#include "dwgrows.h"
#include "vcmap.h"
#include "vcxstrm.h"


/* Compresses the contents of input_buffer into output_buffer.  All
   packets compressed using this function will form a single
   compressed data stream; however, data will be flushed at the end of
   every call so that each output_buffer can be decompressed
   independently (but in the appropriate order since they together
   form a single compression stream) by the receiver.  This appends
   the compressed data to the output buffer. */

static
void
buffer_compress(z_stream& z, char *input_buffer, int len, DwGrowingString *out)
{
  char buf[4096];
  int status;

  /* This case is not handled below. */
  if (len == 0)
    return;

  /* Input is the contents of the input buffer. */
  z.next_in = (unsigned char *)input_buffer;
  z.avail_in = len;

  /* Loop compressing until deflate() returns with avail_out != 0. */
  do
    {
      /* Set up fixed-size output buffer. */
      z.next_out = (unsigned char *)buf;
      z.avail_out = sizeof(buf);

      /* Compress as much data into the buffer as possible. */
      status = deflate(&z, Z_PARTIAL_FLUSH);
      switch (status)
	{
	case Z_OK:
	  /* Append compressed data to output_buffer. */
	 out->append(buf, sizeof(buf) - z.avail_out);
	  break;
	default:
	  USER_BOMB2("compression failed");
	}
    }
  while (z.avail_out == 0);
}

/* Uncompresses the contents of input_buffer into output_buffer.  All
   packets uncompressed using this function will form a single
   compressed data stream; however, data will be flushed at the end of
   every call so that each output_buffer.  This must be called for the
   same size units that the buffer_compress was called, and in the
   same order that buffers compressed with that.  This appends the
   uncompressed data to the output buffer. */

static
void
buffer_uncompress(z_stream& z, char *input_buffer, int len, DwGrowingString *out)
{
  char buf[4096];
  int status;

  z.next_in = (unsigned char *)input_buffer;
  z.avail_in = len;

  z.next_out = (unsigned char *)buf;
  z.avail_out = sizeof(buf);

  for (;;)
    {
      status = inflate(&z, Z_PARTIAL_FLUSH);
      switch (status)
	{
	case Z_OK:
	  out->append(buf, sizeof(buf) - z.avail_out);
	  z.next_out = (unsigned char *)buf;
	  z.avail_out = sizeof(buf);
	  break;
	case Z_BUF_ERROR:
	  /* Comments in zlib.h say that we should keep calling inflate()
	     until we get an error.  This appears to be the error that we
	     get. */
	  return;
	default:
	  USER_BOMB2("bad uncompress");
	}
    }
}


static
void
compress_dtor(long ctx)
{
	if(!ctx) return;
	z_stream *d = (z_stream *)ctx;
	deflateEnd(d);
	delete d;
}

static
void
decompress_dtor(long ctx)
{
	if(!ctx) return;
	z_stream *d = (z_stream *)ctx;
	inflateEnd(d);
	delete d;
}

vc
vclh_compression_open(vc level)
{
	if(level.type() != VC_INT)
	{
		USER_BOMB("first arg is compression level [1..9]", vcnil);
	}
	z_stream *d = new z_stream;
	memset(d, 0, sizeof(*d));
	deflateInit2(d, (int)level, Z_DEFLATED, 9, 4, Z_DEFAULT_STRATEGY);
	return vc(VC_INT_DTOR, (const char *)compress_dtor, (long)d);
}

vc
vclh_compression_close(vc ctx)
{
	ctx.close();
	return vctrue;
}

vc
vclh_decompression_open()
{
	z_stream *d = new z_stream;
	memset(d, 0, sizeof(*d));
	inflateInit(d);
	return vc(VC_INT_DTOR, (const char *)decompress_dtor, (long)d);
}

vc
vclh_decompression_close(vc ctx)
{
	ctx.close();
	return vctrue;
}

static
DwGrowingString *
run_cfilter(z_stream& z, const char *str, int len)
{
	DwGrowingString *ds = new DwGrowingString(len);
	buffer_compress(z, (char *)str, len, ds);
	return ds;
}

static
DwGrowingString *
run_dfilter(z_stream& z, const char *str, int len)
{
	DwGrowingString *ds = new DwGrowingString(2 * len);
	buffer_uncompress(z, (char *)str, len, ds);
	return ds;
}

vc
vclh_compress(vc ctx, vc str)
{
	z_stream *d = (z_stream *)(long)ctx;
	int n;
	DwGrowingString *ds = run_cfilter(*d, (const char *)str, str.len());
	vc ret(VC_BSTRING, ds->ref_str(), ds->length());
	delete ds;
	return ret;
}

vc
vclh_decompress(vc ctx, vc str)
{
	z_stream *d = (z_stream *)(long)ctx;
	DwGrowingString *ds = run_dfilter(*d, (const char *)str, str.len());
	vc ret(VC_BSTRING, ds->ref_str(), ds->length());
	delete ds;
	return ret;
}

vc
vclh_compress_xfer(vc ctx, vc v)
{
	vcxstream vcx((char *)0, (long)128 * 1024, vcxstream::CONTINUOUS);
	long len;
	vc_composite::new_dfs();
	if(!vcx.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
		return vcnil;
	if((len = v.xfer_out(vcx)) == -1)
		return vcnil;

	z_stream *d = (z_stream *)(long)ctx;
	DwGrowingString *b = run_cfilter(*d, vcx.buf, vcx.cur - vcx.buf);
	vc ret(VC_BSTRING, b->ref_str(), b->length());
	delete b;
	return ret;
}

vc
vclh_decompress_xfer(vc ctx, vc v, vc& out)
{
	z_stream *d = (z_stream *)(long)ctx;
	DwGrowingString *b = run_dfilter(*d, (const char *)v, v.len());

	vcxstream vcx((const char *)b->ref_str(), b->length(), vcxstream::FIXED);

	vc item;
	long len;
	if(!vcx.open(vcxstream::READABLE))
	{
		delete b;
		return vcnil;
	}
	if((len = item.xfer_in(vcx)) < 0)
	{
		delete b;
		return vcnil;
	}
	out = item;
	delete b;
	return vctrue;
}

vc
lh_decompress_xfer(vc ctx, vc v, vc out)
{
	if(out.type() != VC_STRING)
	{
		USER_BOMB("third arg to decom-xfer must be a string to bind to", vcnil);
	}
	vc dec;
	if(vclh_decompress_xfer(ctx, v, dec).is_nil())
		return vcnil;
	Vcmap->local_add(out, dec);
	return vctrue;
}
