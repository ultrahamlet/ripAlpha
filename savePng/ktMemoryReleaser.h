#pragma once

class ktMemoryReleaser{
public:
	ktMemoryReleaser(void *Mem){
		this->Mem = Mem;
	}
	~ktMemoryReleaser(){
		free(Mem);
	}
private:
	void *Mem;
};