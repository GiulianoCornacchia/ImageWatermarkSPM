#include "IW_par_functions.cpp"


//gloabal variables

int n_img=0;
int nw;
bool avg=true;

img_t image_watermark;
std::string source_path;
std::string destination_path;


queue<std::string> emitter_queue;
std::vector<queue<std::pair<std::string, img_t>>> queue_stage12;
std::vector<queue<std::pair<std::string, img_t>>> queue_stage23;


/*

EMITTER: first reads the path od the images and then put the paths in the shared data structure "emitter_queue".

STAGE1: reads the path from the "emitter_queue", load the image and puts the pair <image_name,image> in "queue_stage12".

STAGE2: reads the image from the "queue_stage12", aplly the watermark and puts the pair <image_name,image> in "queue_stage23".

STAGE3: reads the image from the "queue_stage23" and save it to the disk.

*/

void emitter(int parallel_degree)
{

	struct dirent *entry;
	

	DIR *dir = opendir(source_path.c_str());


	if(dir==NULL)
	{
		for(int i=0;i<parallel_degree;i++)
			emitter_queue.push("EOS");
		return ;
	}

	auto start = std::chrono::high_resolution_clock::now();

	while((entry = readdir(dir))!=NULL)
	{
		std::string s(entry->d_name);
		
		if(s.compare(".")!=0 && s.compare("..")!=0 )
		{
			emitter_queue.push(s);
			n_img++;			
		}
	}

	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto tot_emitter_ms  = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	auto tot_emitter_ns  = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();

/*
	std::cout<<"Emitter spent "<<tot_emitter_ns<<" ns"<<std::endl;
	std::cout<<"Emitter spent "<<tot_emitter_ms<<" ms"<<std::endl;
	*/


	for(int i=0;i<parallel_degree;i++)
		emitter_queue.push("EOS");

	closedir(dir);

	return;
}


void stage1(int i)
{
	std::string image_path;
	std::string image_name;
	bool eos=false;
	int my_img=0;
	int tot=0;


	
	while(!eos)
	{
		//read

		
		image_name = emitter_queue.pop();
		

		if(image_name.compare("EOS")!=0)
		{
			//loading image
			

			auto start = std::chrono::high_resolution_clock::now();
			std::string image_path = source_path+"/"+image_name;
			img_t img(image_path.c_str());
			auto elapsed1 = std::chrono::high_resolution_clock::now() - start;
	   	    auto ms1  = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed1).count();
	    	tot+=ms1;

			//pushing into queue_stage12
			std::pair<std::string, img_t> p(image_name, img);
			queue_stage12[i].push(p);

			my_img++;

		}
		else
		{	
			eos=true;

			//PUSH EOS
			std::pair<std::string, img_t> p("EOS", img_t());
			queue_stage12[i].push(p);

			//std::cout<<i<<" "<<my_img<<std::endl;

		}
		
		
	  
	}
	  
	  
	  //std::cout<<"Stage 1 ["<<i<<"] processed "<<my_img<<" in "<<tot<<" msec  AVG: "<<tot/my_img<<" ms"<<std::endl;

	return;
		
}
	

void stage2(int i)
{
	//extract the image and apply the watermark
	bool eos=false;
	int my_img=0;
	int tot=0;
	std::pair<std::string, img_t> p_ext;


	
	while(!eos)
	{

		p_ext = queue_stage12[i].pop();
		

		if(p_ext.first.compare("EOS")!=0)
		{

			auto start = std::chrono::high_resolution_clock::now();
			only_watermark(image_watermark,p_ext.second,avg);
			auto elapsed2 = std::chrono::high_resolution_clock::now() - start;
	  		auto ms2  = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed2).count();
	  		tot+=ms2;

			//push on queue_stage23

			std::pair<std::string, img_t> p(p_ext.first, p_ext.second);
			queue_stage23[i].push(p);
			my_img++;
			

		}
		else
		{
			eos=true;

			//PUSH EOS
			std::pair<std::string, img_t> p("EOS", img_t());
			queue_stage23[i].push(p);

		}
		


	}

	  
	  
	// std::cout<<"Stage 2 ["<<i<<"] processed "<<my_img<<" in "<<tot<<" msec  AVG: "<<tot/my_img<<" ms"<<std::endl;

	return;

}


void stage3(int i)
{
	//extract the image and save
	bool eos=false;

	std::pair<std::string, img_t> p_ext;
	std::string path_save;
	int tot=0;
	int my_img=0;

	
	while(!eos)
	{

		p_ext = queue_stage23[i].pop();
		

		if(p_ext.first.compare("EOS")!=0)
		{

			auto start = std::chrono::high_resolution_clock::now();
			path_save = destination_path+"/"+p_ext.first;

			try{
			p_ext.second.save_jpeg(path_save.c_str(),100);
		    }catch(cimg_library::CImgIOException ex)
		    {

		    }
			auto elapsed3 = std::chrono::high_resolution_clock::now() - start;
		    auto ms3  = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed3).count();
		    tot+=ms3;

			my_img++;


		}
		else	
			eos=true;


		
	}


	 
	  
	//std::cout<<"Stage 3 ["<<i<<"] processed "<<my_img<<" in "<<tot<<" msec  AVG: "<<tot/my_img<<" ms"<<std::endl;

	return;

}



// ./exe source dest wm avg par_deg

int main(int argc, char* argv[])
{
	struct dirent *entry;
	int t_load=0, t_proc=0, t_save=0;

	

	if(argc<6)
	{
		std::cout<<"Wrong parameters, the correct parameters are source_folder destination_folder watermark avg par_deg"<<std::endl;
		return 0;
	}

	image_watermark = img_t(argv[3]);
	source_path = std::string(argv[1]);
	destination_path = std::string(argv[2]);


	if(atoi(argv[4])==0)
		avg=false;
	else if(atoi(argv[4])==1)
		avg=true;
	else
	{
		std::cout<<"Wrong value for Avg, try with 0 or 1"<<std::endl;
		return 0;

	}

	nw=atoi(argv[5]);

	if(nw<=0)
	{
		std::cout<<"Error, the parallel_degree MUST be positive"<<std::endl;
		return 0;
	}

	//std::cout<<nw<<std::endl;


	auto start_sd = std::chrono::high_resolution_clock::now();
	std::vector<std::thread> threads;

	queue_stage12=std::vector<queue<std::pair<std::string, img_t>>> (nw);
	queue_stage23=std::vector<queue<std::pair<std::string, img_t>>> (nw);

	auto elapsed_sd = std::chrono::high_resolution_clock::now() - start_sd;
	auto n_sec_sd = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_sd).count();
	auto m_sec_sd = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_sd).count();
	//std::cout<<n_sec_sd<<std::endl;
	

	//starting point
	auto start = std::chrono::high_resolution_clock::now();
	for(int i=0;i<nw;i++)
	{
		threads.push_back(std::thread(stage1,i));
		threads.push_back(std::thread(stage2,i));
		threads.push_back(std::thread(stage3,i));
	}

	threads.push_back(std::thread(emitter,nw));

	auto elapsed_th = std::chrono::high_resolution_clock::now() - start;
	auto n_sec_th = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_th).count();
	auto m_sec_th = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_th).count();
	//std::cout<<"Created "<<nw*3+1<<" threads in "<<n_sec_th<<" nsec     "<<m_sec_th<<" msec"<<std::endl;
	


	for(std::thread& t: threads)
		t.join();	

	auto elapsed = std::chrono::high_resolution_clock::now() - start;

	auto m_sec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cout<<"Parallel degree: "<<nw<<std::endl;
	std::cout<<"Processed "<<n_img<<" images in "<<m_sec<<" msec."<<std::endl;

	return 0;
}


