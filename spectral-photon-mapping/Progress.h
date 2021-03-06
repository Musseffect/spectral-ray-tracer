#pragma once
#include <stdio.h>

class Progress {
public:
	virtual void emitProgress(float progress) = 0;
};

class ConsoleProgress : public Progress {
public:
	void emitProgress(float progress) override
	{
		printf("\33[2K\rProgress %3.3f %% \r", progress);
	}
};