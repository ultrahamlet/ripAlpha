#pragma once

class ktFileCloser
{
public:
	ktFileCloser(FILE *fp){
		this->fp = fp;
	}
	~ktFileCloser(){
		fclose(fp);
	}
private:
	FILE *fp;
};
