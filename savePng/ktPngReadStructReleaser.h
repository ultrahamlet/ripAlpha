#pragma once

class ktPngReadStructReleaser{
public:
	ktPngReadStructReleaser(png_struct *p, png_info *q){
		this->p = p;
		this->q = q;
	}
	~ktPngReadStructReleaser(){
		png_destroy_read_struct(&p, &q, NULL);
	}
private:
	png_struct *p;
	png_info *q;
};