/*
 * make_map.c
 *
 *  Created on: 2016年1月19日
 *      Author: Random
 */
#include <stdio.h>
#include <string.h>

// #include <wininet.h>
#include <windows.h>

#include <setjmp.h>
#define XMD_H
#include "include/libjpeg/jpeglib.h"

#define NAME_LEN 64
#define INT32 int

struct map_name_templete
{
	char		temp[NAME_LEN];
	char		new_temp[NAME_LEN];
	int			x;
	int			y;
	int			m;

	int			xSize;
	int			ySize;
	int			mapID;

	int			output_components;
	int			output_width;
	int			output_height;

	FILE		*output_file;

	char		is_write_head;


	unsigned char*buff;
	int			buff_size;

	int			out_color_space;
	unsigned char quantize_colors;

	unsigned char		density_unit;		/* JFIF code for pixel size units */
	unsigned short		X_density;		/* Horizontal pixel density */
	unsigned short		Y_density;		/* Vertical pixel density */
};

void map_name_template_init(struct map_name_templete* temp)
{
	char* c;
	temp->x = NAME_LEN + 1;
	temp->y = NAME_LEN + 1;
	temp->m = NAME_LEN + 1;
	memcpy(temp->new_temp, temp->temp, NAME_LEN);
	c = strstr(temp->new_temp, "%X");
	if(c)
	{
		temp->x = (int)(c - temp->new_temp);
		*(c + 1) = 'd';
	}
	c = strstr(temp->new_temp, "%Y");
	if(c)
	{
		temp->y = (int)(c - temp->new_temp);
		*(c + 1) = 'd';
	}
	c = strstr(temp->new_temp, "%M");
	if(c)
	{
		temp->m = (int)(c - temp->new_temp);
		*(c + 1) = 'd';
	}
}

char get_jpeg_file_name(char* name, struct map_name_templete* temp, int xIndex, int yIndex, int mapID)
{
	int i, j;
	int c;
	int l1[3] = {xIndex, yIndex, mapID};
	int l2[3] = {temp->x, temp->y, temp->m};

	for(i = 0; i < 2; ++i)
	{
		for(j = 0; j < 2; ++j)
		{
			if(l2[j] > l2[j + 1])
			{
				c = l2[j];
				l2[j] = l2[j + 1];
				l2[j + 1] = c;

				c = l1[j];
				l1[j] = l1[j + 1];
				l1[j + 1] = c;
			}
		}
	}
	if(l1[2] < sizeof(temp->new_temp))
	{
		sprintf(name, temp->new_temp, l1[0], l1[1], l1[2]);
		return 1;
	}
	if(l1[1] < sizeof(temp->new_temp))
	{
		sprintf(name, temp->new_temp, l1[0], l1[1]);
		return 1;
	}
	if(l1[0] < sizeof(temp->new_temp))
	{
		sprintf(name, temp->new_temp, l1[0]);
		return 1;
	}
	return 0;
}



char read_jpg_file(struct map_name_templete* temp, const char* input_filename, int xIndex, int xSize)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *input_file;
	JSAMPARRAY buffer;
	int width;
	int row_width;
	int row_index;

	cinfo.err = jpeg_std_error(&jerr);

//	printf("read jpg file %s\n", input_filename);
	fflush(stdout);

	if ((input_file = fopen(input_filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", input_filename);
		return 0;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, input_file);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	row_width = cinfo.output_width * cinfo.output_components;

	if(temp->buff_size < (row_width * cinfo.output_height * xSize))
	{
		free(temp->buff);
		temp->buff = 0;
		temp->buff_size = 0;

		temp->output_width = cinfo.output_width;
		temp->output_height = cinfo.output_height;
		temp->output_components = cinfo.output_components;
		temp->out_color_space = cinfo.out_color_space;
		temp->quantize_colors = cinfo.quantize_colors;
		temp->density_unit = cinfo.density_unit;
		temp->X_density = cinfo.X_density;
		temp->Y_density = cinfo.Y_density;

		temp->buff_size = row_width * temp->output_height * xSize;
		temp->buff = (unsigned char*)malloc(temp->buff_size);
		if(!temp->buff)
		{
			temp->buff_size = 0;
			fclose(input_file);
			return 0;
		}
		memset(temp->buff, 0, temp->buff_size);
	}
	if(temp->buff )
	{
		buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE,
				row_width, 1);
		row_index = 0;
		width = temp->output_width * temp->output_components;
		while(cinfo.output_scanline < cinfo.output_height)
		{
			if(row_index >= cinfo.output_height) break;
			jpeg_read_scanlines(&cinfo, buffer, 1);
//			memcpy(temp->buff  + width * row_index, *buffer, row_width);
//			printf("memcpy %d %d %d %d %d %d %d %d %d\n",temp->output_height, temp->output_width,temp->buff_size,   width, row_index, xSize, xIndex, width * (row_index * xSize + xIndex), row_width <= width? row_width : width);
//			fflush(stdout);
			memcpy((char*)(temp->buff + width * (row_index * xSize + xIndex)), *buffer ,row_width <= width? row_width : width);
			++row_index;
		}
	}
//	 1 sdjpg tt3.bmp "%Y_%X[%M].jpg" 20
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(input_file);
//	printf("read jpg file %s end\n", input_filename);
	fflush(stdout);
	return 1;
}

#define PUT_2B(array,offset,value)  \
        (array[offset] = (char) ((value) & 0xFF), \
         array[offset+1] = (char) (((value) >> 8) & 0xFF))
#define PUT_4B(array,offset,value)  \
        (array[offset] = (char) ((value) & 0xFF), \
         array[offset+1] = (char) (((value) >> 8) & 0xFF), \
         array[offset+2] = (char) (((value) >> 16) & 0xFF), \
         array[offset+3] = (char) (((value) >> 24) & 0xFF))

void write_bmp_header(struct map_name_templete* temp) {
	char bmpfileheader[14];
	char bmpinfoheader[40];
	long headersize, bfSize;
	int bits_per_pixel, cmap_entries;
	int i;
	int step;

	/* Compute colormap size and total file size */
	if (temp->out_color_space == JCS_RGB) {
		printf("-=-----------------------%d\n", temp->quantize_colors);
		if (temp->quantize_colors) {
			/* Colormapped RGB */
			bits_per_pixel = 8;
			cmap_entries = 256;
		} else {
			/* Unquantized, full color RGB */
			bits_per_pixel = 24;
			cmap_entries = 0;
		}
	} else {
		/* Grayscale output.  We need to fake a 256-entry colormap. */
		bits_per_pixel = 8;
		cmap_entries = 256;
	}

	step = temp->output_width * temp->output_components * temp->xSize;

	while ((step & 3) != 0)
		step++;

	/* File size */
	headersize = 14 + 40 + cmap_entries * 4; /* Header and colormap */

	bfSize = headersize + (long) step * (long) temp->output_height;

	/* Set unused fields of header to 0 */
	memset(bmpfileheader, 0, sizeof(bmpfileheader));
	memset(bmpinfoheader, 0, sizeof(bmpinfoheader));

	/* Fill the file header */
	bmpfileheader[0] = 0x42;/* first 2 bytes are ASCII 'B', 'M' */
	bmpfileheader[1] = 0x4D;
	PUT_4B(bmpfileheader, 2, bfSize); /* bfSize */
	/* we leave bfReserved1 & bfReserved2 = 0 */
	PUT_4B(bmpfileheader, 10, headersize); /* bfOffBits */

	/* Fill the info header (Microsoft calls this a BITMAPINFOHEADER) */
	PUT_2B(bmpinfoheader, 0, 40); /* biSize */
	PUT_4B(bmpinfoheader, 4, temp->output_width * temp->xSize); /* biWidth */
	PUT_4B(bmpinfoheader, 8, temp->output_height * temp->ySize); /* biHeight */
	PUT_2B(bmpinfoheader, 12, 1); /* biPlanes - must be 1 */
	PUT_2B(bmpinfoheader, 14, bits_per_pixel); /* biBitCount */
	/* we leave biCompression = 0, for none */
	/* we leave biSizeImage = 0; this is correct for uncompressed data */
	if (temp->density_unit == 2) { /* if have density in dots/cm, then */
		PUT_4B(bmpinfoheader, 24, (INT32 ) (temp->X_density * 100)); /* XPels/M */
		PUT_4B(bmpinfoheader, 28, (INT32 ) (temp->Y_density * 100)); /* XPels/M */
	}
	PUT_2B(bmpinfoheader, 32, cmap_entries); /* biClrUsed */
	/* we leave biClrImportant = 0 */

	if (fwrite(bmpfileheader, 1, 14, temp->output_file) != (size_t) 14) {
		printf("write bmpfileheader error\n");
	}
	if (fwrite(bmpinfoheader, 1, 40, temp->output_file) != (size_t) 40) {
		printf("write bmpinfoheader error\n");
	}

	for(i = 0; i < sizeof(bmpinfoheader); ++i)
	{
		printf("%d ", (int)bmpinfoheader[i]);
	}
	printf("\n");

}

void write_pixel_data(struct map_name_templete* temp) {
	int rows, cols;
	int row_width;
	int step;
	unsigned char *tmp = NULL;

	unsigned char *pdata;

	row_width = temp->output_width * temp->output_components * temp->xSize;
	step = row_width;
	while ((step & 3) != 0)
		step++;

	pdata = (unsigned char *) malloc(step);
	memset(pdata, 0, step);

	tmp = temp->buff + row_width * (temp->output_height - 1);
	for (rows = 0; rows < temp->output_height; rows++) {
		for (cols = 0; cols < row_width; cols += 3) {
			pdata[cols + 2] = tmp[cols + 0];
			pdata[cols + 1] = tmp[cols + 1];
			pdata[cols + 0] = tmp[cols + 2];
		}
		tmp -= row_width;
		fwrite(pdata, 1, step, temp->output_file);
	}
	free(pdata);
}

char read_row_files(const char* inpath, struct map_name_templete* temp, int xSize, int yIndex, int mapID)
{
	char name[NAME_LEN];
	char fullpath[NAME_LEN * 2];
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	size_t size;
	int i;

	size = strlen(inpath);
	memcpy(fullpath, inpath, size + 1);


	if(fullpath[size - 1] != '\\' && fullpath[size - 1] != '/')
	{
		fullpath[size] = '/';
		size += 1;
		fullpath[size] = 0;
	}

	if(temp->buff)
	{
		memset(temp->buff, 0, temp->buff_size);
	}
	for(i = 0; i < xSize; ++i)
	{
		get_jpeg_file_name(name, temp, i, yIndex, mapID);
		fullpath[size] = 0;
		strcat(fullpath, name);
		hFind = FindFirstFile(fullpath, &wfd);
		if(hFind == INVALID_HANDLE_VALUE)
		{
			printf("can't find file %s     %d %d %d\n", fullpath, i, yIndex, mapID);
			continue;
		}
		read_jpg_file(temp, fullpath, i, xSize);
	}
	if(temp->buff)
	{
		if(!temp->is_write_head)
		{
			temp->is_write_head = 1;
			write_bmp_header(temp);
		}
		write_pixel_data(temp);
	}
	return 1;

}


char
make_map(const char* inpath, const char* output_filename, struct map_name_templete* temp)
{
	int i;
	FILE *output_file;
	if ((output_file = fopen(output_filename, "wb")) == NULL) {

		fprintf(stderr, "can't open %s\n", output_filename);
		return 0;
	}
	temp->output_file = output_file;
	for(i = temp->ySize - 1 ; i >= 0 ; --i)
	{
		read_row_files(inpath, temp, temp->xSize, i, temp->mapID);
	}
	if(temp->buff)
	{
		free(temp->buff);
		temp->buff_size = 0;
	}
	fclose(output_file);
	return 1;
}

int main(int argc, char** argv)
{
	const char* temp;
	char name[NAME_LEN];
	const char* outfile;
	const char* indir;
	int i, j;
	int maxCnt;

	char fullpath[NAME_LEN * 2];
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	size_t size;

	int savei, savej;

	savei = 0;
	savej = 0;

	struct map_name_templete t;

	memset(&t, 0, sizeof(t));
	temp = argv[4];
	memcpy(t.temp, temp, strlen(temp));
	map_name_template_init(&t);

	t.mapID = atoi(argv[1]);
	indir = argv[2];
	outfile = argv[3];
	maxCnt = 4;
	if (argc > 5)
	{
		maxCnt = atoi(argv[5]);
	}


	memset(fullpath, 0, sizeof(fullpath));
	size = strlen(indir);
	memcpy(fullpath, indir, size + 1);

	if(fullpath[size - 1] != '\\' && fullpath[size - 1] != '/')
	{
		fullpath[size] = '/';
		size += 1;
		fullpath[size] = 0;
	}
//	printf("check file, %s %d\n", fullpath, maxCnt);
	for(i = 0; i < maxCnt; ++i)
	{
		for(j = 0; j < maxCnt; ++ j)
		{
			fullpath[size] = 0;

			get_jpeg_file_name(name, &t, i, j, t.mapID);
			strcat(fullpath, name);
//			printf("check file, %s  %s  %s\n", fullpath, t.new_temp, t.temp);
			hFind = FindFirstFile(fullpath, &wfd);
			if(hFind == INVALID_HANDLE_VALUE)
			{
				continue;
			}
			if(!savei)
			{
				savei = i;
				savej = j;
			}
			if(i >= t.xSize)
			{
				t.xSize = i + 1;
			}
			if(j >= t.ySize)
			{
				t.ySize = j + 1;
			}

		}
	}

	if(!t.buff)
	{
		fullpath[size] = 0;
		get_jpeg_file_name(name, &t, savei, savej, t.mapID);
		strcat(fullpath, name);
		read_jpg_file(&t, fullpath, savei, t.xSize);
		printf("ready buff %d %d %d\n", t.buff_size, savei, savej);
	}

	printf("go %d %d \n", t.xSize, t.ySize);

	make_map(indir, outfile, &t);
	return 0;
}
