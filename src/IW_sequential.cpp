#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include "CImg.h"
#include <chrono>


using img_t = cimg_library::CImg<int>;

void apply_watermark(const char* save_name, img_t watermark, img_t img, bool transparent, int* t_proc, int* t_save)
{
	int w,h,ch,ch2;


	w = watermark.width();
	h = watermark.height();
	ch = watermark.spectrum();
	ch2 = img.spectrum();


	auto start = std::chrono::high_resolution_clock::now();
	for(int i=0;i<w;i++)	
		for(int j=0;j<h;j++)
			if((watermark(i,j,0,0)<10))
				for(int c=0;c<ch2;c++)
					if(transparent)
						img(i,j,0,c)=img(i,j,0,c)/2;
					else			
						img(i,j,0,c)=0;
	auto elapsed = std::chrono::high_resolution_clock::now() - start;				
	auto m_sec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	*t_proc+=(int)m_sec;


	start = std::chrono::high_resolution_clock::now();
	img.save_jpeg(save_name,100);
	elapsed = std::chrono::high_resolution_clock::now() - start;
	m_sec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	*t_save+=(int)m_sec;
	
}



int main(int argc, char* argv[])
{
	struct dirent *entry;
	int n_img=0;
	int t_load=0, t_proc=0, t_save=0;

	if(argc<5)
	{
		std::cout<<"Wrong parameters, the correct parameters are source_folder destination_folder watermark avg"<<std::endl;
		return 0;
	}

    std::string in_ = argv[1];
    std::string out_= argv[2];
    img_t wm(argv[3]);
    bool avg=true;
	DIR *dir = opendir(in_.c_str());

	

	if(atoi(argv[4])==0)
		avg=false;
	else if(atoi(argv[4])==1)
		avg=true;
	else
	{
		std::cout<<"Wrong value for Avg, try with 0 or 1"<<std::endl;
		return 0;
	}


	if(dir==NULL)
		return -1;

	


	//starting point
	auto start = std::chrono::high_resolution_clock::now();


	//reading the paths
	while((entry = readdir(dir))!=NULL)
	{
		auto start2 = std::chrono::high_resolution_clock::now();
		std::string s(entry->d_name);
		
		if(s.compare(".")!=0 && s.compare("..")!=0 )
		{
			std::string path_in = in_+"/"+s;
			std::string path_save = out_+"/"+s;
			img_t img(path_in.c_str());
			auto elapsed2 = std::chrono::high_resolution_clock::now() - start2;
			auto m_sec2 = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed2).count();
			t_load+=(int)m_sec2;
			apply_watermark(path_save.c_str(),wm,img,avg,&t_proc,&t_save);
			n_img++;
			
		}

	}

	auto elapsed = std::chrono::high_resolution_clock::now() - start;

	auto m_sec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();


	
	std::cout<<"Processed "<<n_img<<" images in "<<m_sec<<" msec."<<std::endl;

	return 0;
}