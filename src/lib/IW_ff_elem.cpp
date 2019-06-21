#include "IW_par_functions.cpp"
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>


using namespace ff;


//global variables
std::string source_path;
std::string destination_path;
img_t image_watermark;
int n_img=0;
bool avg=true;


//the svc of each node contains the same function of the c++ implementation


struct emitter: ff_node_t<std::string>
{

	std::string* svc(std::string *gc)
	{
		struct dirent *entry;
		DIR *dir = opendir(source_path.c_str());

		if(dir==NULL)
		{
			return EOS;
		}

		while((entry = readdir(dir))!=NULL)
		{
			std::string s(entry->d_name);
			
			if(s.compare(".")!=0 && s.compare("..")!=0 )
			{
				std::string *name = new std::string(s);
				ff_send_out(name);
				n_img++;			
			}
		}


	closedir(dir);

	return EOS;


	}
	
};



struct stage1: ff_node_t<std::string, std::pair<std::string, img_t*>>
{


	std::pair<std::string, img_t*>* svc(std::string *tsk)
	{

			std::string image_path;
			std::string &image_name=*tsk;
			img_t* img;

		
			image_path = source_path+"/"+image_name;

			try{
			img = new img_t(image_path.c_str());
		    }catch(cimg_library::CImgIOException ex)
		    {}
		

			//create pair
			std::pair<std::string, img_t*>*p12= new std::pair<std::string, img_t*>(image_name, img);
			
			delete tsk;
			return p12;	
			
	}

};



struct stage2: ff_node_t<std::pair<std::string, img_t*>>
{

	std::pair<std::string, img_t*>* svc(std::pair<std::string, img_t*> *tsk)
	{

		std::pair<std::string, img_t*> &p = *tsk;

		

		only_watermark(image_watermark,*(p.second),avg);
	

		return tsk;
		
	}

};



struct stage3: ff_node_t<std::pair<std::string, img_t*>>
{

	std::string path_save;

	std::pair<std::string, img_t*>* svc(std::pair<std::string, img_t*> *tsk)
	{

		std::pair<std::string, img_t*> &p = *tsk;

		path_save = destination_path+"/"+p.first;

		try{
			(*(p.second)).save_jpeg(path_save.c_str(),100);
		    }catch(cimg_library::CImgIOException ex)
		    {
		    }

	 delete p.second;
	 delete tsk;
	 return GO_ON;	    
	}
	
};