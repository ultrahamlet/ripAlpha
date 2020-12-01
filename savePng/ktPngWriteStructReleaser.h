#pragma once

class ktPngWriteStructReleaser{
public:
	ktPngWriteStructReleaser(png_struct *p, png_info *q){
		this->p = p;
		this->q = q;
	}
	~ktPngWriteStructReleaser(){
		png_destroy_write_struct(&p, &q);
	}
private:
	png_struct *p;
	png_info *q;
};