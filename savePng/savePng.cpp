// savePng.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "png.h"
#include "ktFileCloser.h"
#include "ktMemoryReleaser.h"
#include "ktPngReadStructReleaser.h"
#include "ktPngWriteStructReleaser.h"
#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL



void PngReadFunc(png_struct *pPng, png_bytep buf, png_size_t size){
	/*ただのバイト列なのに・・・*/
	unsigned char** p = (unsigned char**)png_get_io_ptr(pPng);
	memcpy(buf, *p, size);
	*p += (int)size;
}

int loadPngFile(char* argv1)
{

	FILE *fp = fopen(argv1, "rb");
	if(!fp) return 0;
	ktFileCloser pngfilecloser(fp);
	int fsize;
	unsigned char *FileImage = (unsigned char*)malloc((fseek(fp, 0, SEEK_END), fsize = ftell(fp)));
	ktMemoryReleaser fileimagereleaer(FileImage);
	fseek(fp, 0, SEEK_SET);
	fread(FileImage, fsize, 1, fp);
	
	if(!png_check_sig(FileImage, fsize))
		return 0; // pngでない！
	
	png_struct *pPng;
	png_info *pInfo;

		if(!(pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
		return 0; // エラー
	if(!(pInfo = png_create_info_struct(pPng)))
		return 0;
	ktPngReadStructReleaser pngreleaser(pPng, pInfo);

	unsigned char* filepos = FileImage;
	png_set_read_fn(pPng,(png_voidp)&filepos,(png_rw_ptr)PngReadFunc);

	png_read_info(pPng, pInfo);

	png_uint_32 PngWidth;
	png_uint_32 PngHeight;
	int bpp;
	int ColorType;

	png_get_IHDR(pPng, pInfo, &PngWidth, &PngHeight, &bpp, &ColorType, NULL, NULL, NULL);

	//if (ColorType == PNG_COLOR_TYPE_RGB) printf("PNG_COLOR_TYPE_RGB");
    //if (ColorType == PNG_COLOR_TYPE_RGB_ALPHA) printf("PNG_COLOR_TYPE_RGB_ALPHA");
	
	if(png_get_valid(pPng, pInfo, PNG_INFO_tRNS))
		png_set_expand(pPng);
	if(ColorType == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(pPng);
	if(ColorType == PNG_COLOR_TYPE_GRAY && bpp < 8)
		png_set_expand(pPng);
	if(bpp > 8)
		png_set_strip_16(pPng);
	if(ColorType == PNG_COLOR_TYPE_GRAY)
		png_set_gray_to_rgb(pPng);

	unsigned char *BmpBuffer;
	unsigned char **Lines;
	BmpBuffer = (unsigned char *)malloc(PngWidth * PngHeight * 4);
	Lines = (unsigned char **)malloc(sizeof(unsigned char *) * PngHeight);
	for(int i = 0; i < (int)PngHeight; i++)Lines[i] = BmpBuffer + PngWidth * 4 * i;
	if(!(ColorType & PNG_COLOR_MASK_ALPHA))png_set_filler(pPng, 255, PNG_FILLER_AFTER);
	png_set_bgr(pPng);
	png_read_image(pPng, Lines);
    //printf("%d %d %d %d\n",*(BmpBuffer+0), *(BmpBuffer+1),*(BmpBuffer+2),*(BmpBuffer+3));
	//printf("%d %d %d %d\n",*(BmpBuffer+4), *(BmpBuffer+5),*(BmpBuffer+6),*(BmpBuffer+7));
	//printf("%d %d %d %d\n",*(BmpBuffer+8), *(BmpBuffer+9),*(BmpBuffer+10),*(BmpBuffer+11));
	free(Lines);
	for (int i = 0; i < (int)PngWidth * (int)PngHeight * 4; i+=4){
        //if ((i % 5) == 0) *(BmpBuffer+i) = 128;
		//*(BmpBuffer + i + 3) = 120;
	}
	// -------------------------------------------------------------
	char buf[256];
	sprintf(buf, "out/%s", argv1);
    fp = fopen(buf, "wb");
	if(!fp)return 0;
	ktFileCloser ktfileclose(fp);
	png_structp png_ptr;
	png_infop info_ptr;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) return 0;
	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		png_destroy_write_struct(&png_ptr, png_infopp_NULL);
		return 0;
	}
	ktPngWriteStructReleaser pngwritereleaser(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))){
		// エラーの時ここに来る。
		//SelectObject(hDC, hOldBmp);
		//DeleteDC(hDC);
		return 0;
	}

	png_init_io(png_ptr, fp);
	//png_set_IHDR(png_ptr, info_ptr, PngWidth, PngHeight, 8, PNG_COLOR_TYPE_RGB_ALPHA,
	//	PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);
	png_set_IHDR(png_ptr, info_ptr, PngWidth, PngHeight, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

	png_color_8 sig_bit;
	sig_bit.red = 8;
	sig_bit.green = 8;
	sig_bit.blue = 8;
	sig_bit.alpha = 0;
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);

	// PNGに書き込まれるコメント
	png_text text_ptr[1];
	text_ptr[0].key = "Description";
	text_ptr[0].text = "ktcDIB::Save() Data";
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	png_set_text(png_ptr, info_ptr, text_ptr, 1);

	png_write_info(png_ptr, info_ptr);

	png_set_bgr(png_ptr);

	png_bytep row;
	png_bytep *row_pointers;
	row_pointers = (png_bytep *)malloc(sizeof(png_bytep *) * PngHeight);
	//for (int k = 0; k < (int)PngHeight; k++) row_pointers[k] = (png_bytep)(BmpBuffer + PngWidth * 4 * k);

	//
	unsigned char *BmpBuffer2;
	int j = 0;
	BmpBuffer2 = (unsigned char *)malloc(PngWidth * PngHeight * 3);
	for (int i = 0; i < (int)PngWidth * (int)PngHeight * 4; i += 4) {
	
		BmpBuffer2[j] = *(BmpBuffer + i + 0);
	    BmpBuffer2[j+1] = *(BmpBuffer + i + 1);
		BmpBuffer2[j+2] = *(BmpBuffer + i + 2);
		j += 3;
	}
	//
	for (int k = 0; k < (int)PngHeight; k++) row_pointers[k] = (png_bytep)(BmpBuffer2 + PngWidth * 3 * k);
	//
	png_write_image(png_ptr, row_pointers);
	free(row_pointers);
	png_write_end(png_ptr, info_ptr);

	printf("done!\n");
	return 0;
}


#include <Windows.h>

int _tmain(int argc, _TCHAR* argv[])
 
{
	char  c[100];
	//TCHAR型配列をchar型配列に変換
	WideCharToMultiByte(CP_ACP, 0, argv[1], -1, c, sizeof(c), NULL, NULL);
	loadPngFile(c);
	return 0;

}

