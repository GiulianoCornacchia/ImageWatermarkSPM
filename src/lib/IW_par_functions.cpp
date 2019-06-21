#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../CImg.h"
#include <chrono>
#include <deque>



using img_t = cimg_library::CImg<int>;


template <typename T>
class queue
{

		private:
	
		  mutable std::mutex      d_mutex;
		  mutable std::condition_variable d_condition;
		  std::deque<T>           d_queue;
	
		public:
		  queue(){}
		  /*
		  * Constructor overriding to manage non-movable mutex:
		  *
		  * Move Constructor 
		  */
		  queue(queue&& a)
		    {
		        std::unique_lock<std::mutex> lock(a.d_mutex);
		        d_queue = std::move(a.d_queue);
		    }
		  /*
		  * Move Assignment 
		  */
		  queue& operator=(queue&& a)
		    {
		      if (this != &a)
		      {
		        std::unique_lock<std::mutex> lock(d_mutex, std::defer_lock);
		        d_queue = std::move(a.d_queue);
		      }
		      return *this;
		    }
		  /*
		  * Copy Constructor
		  */
		  queue(const queue& a)
		    {
		        std::unique_lock<std::mutex> lock(a.d_mutex);
		        d_queue = a.d_queue;
		    }
		  /*
		  * Copy Assignment 
		  */
		  queue& operator=(const queue& a)
		    {
		        if (this != &a)
		        {
		          std::unique_lock<std::mutex> lock(a.d_mutex, std::defer_lock);
		          d_queue = a.d_queue;
		        }
		        return *this;
		    }
	
		  void push(T value) {
		    {
		      std::unique_lock<std::mutex> lock(this->d_mutex);
		      d_queue.push_front(value);
		    }
		    this->d_condition.notify_one();
		  }
	
		  T pop() {
		    std::unique_lock<std::mutex> lock(this->d_mutex);
		    this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
		    T rc = this->d_queue.back();
		    this->d_queue.pop_back();
		    return rc;
		  }
};



void only_watermark(img_t watermark, img_t &img, bool transparent)
{
	int w,h,w1,h1,ch,ch2;


	w = watermark.width();
	h = watermark.height();
	w1 =img.width();
	h1 =img.height();
	ch = watermark.spectrum();
	ch2 = img.spectrum();



	if(w!=w1 || h!=h1)
	{
		std::cout<<"Error: the watermark and the image MUST have the same size"<<std::endl;

	}
	else
	{

	auto start = std::chrono::high_resolution_clock::now();
	for(int i=0;i<w;i++)	
		for(int j=0;j<h;j++)
			if((watermark(i,j,0,0)<10))
				for(int c=0;c<ch2;c++)
					if(transparent)
						img(i,j,0,c)=img(i,j,0,c)/2;
					else			
						img(i,j,0,c)=0;
	}

}