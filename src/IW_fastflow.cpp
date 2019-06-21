#include "IW_ff_elem.cpp"




/*

EMITTER: first reads the path od the images and then put the paths in the shared data structure "emitter_queue".

STAGE1: reads the path from the "emitter_queue", load the image and puts the pair <image_name,image> in "queue_stage12".

STAGE2: reads the image from the "queue_stage12", aplly the watermark and puts the pair <image_name,image> in "queue_stage23".

STAGE3: reads the image from the "queue_stage23" and save it to the disk.

*/



// ./exe source dest wm avg par_deg

int main(int argc, char* argv[])
{
	
	

	
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

	int nw=atoi(argv[5]);

	if(nw<=0)
	{
		std::cout<<"Error, the parallel_degree MUST be positive"<<std::endl;
		return 0;
	}


	//creating the emitter

	emitter IW_emitter;

	//creating the structure

	std::vector<std::unique_ptr<ff_node>> W;

	
	//create nw pipeline(S1->S2->S3)
	for(int i=0;i<nw;i++)
	{
		//creating the single stages
		stage1* s1 = new stage1();
		stage2* s2 = new stage2();
		stage3* s3 = new stage3();

		//combining S1,S2,S2 in a pipeline

		W.push_back(make_unique<ff_Pipe<>>(s1,s2,s3));

	}


	//creating the farm

	ff_Farm<> IW_farm(std::move(W));

	IW_farm.remove_collector();

	IW_farm.add_emitter(IW_emitter);

	IW_farm.set_scheduling_ondemand();

	

	


	//start

	ffTime(START_TIME);

	if(IW_farm.run_and_wait_end()<0)
	{
		return -1;
	}

	ffTime(STOP_TIME);

	std::cout<<"Parallel degree: "<<nw<<std::endl;
	std::cout<<"Processed "<<n_img<<" images in "<<ffTime(GET_TIME)<<" msec."<<std::endl;

	return 0;
}


