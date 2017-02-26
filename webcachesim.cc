#include <fstream>
#include <regex>
#include "policies/lru_variants.cc"
#include "policies/gd_variants.cc"
#include <chrono>
int main (int argc, char* argv[])
{

  // output help if insufficient params
  if(argc < 5) {
    cerr << "webcachesim traceFile warmUp cacheType log2CacheSize cacheParams" << endl;
    return 1;
  }

  // trace properties
  const char* path = argv[1];
  const long warmUp = atol(argv[2]);
  assert(warmUp>=0);

  // create cache
  const string cacheType = argv[3];
  unique_ptr<Cache> webcache = move(Cache::create_unique(cacheType));
  if(webcache == nullptr)
    return 1;

  // configure cache size
  const double sizeExp = atof(argv[4]);
  cout<<sizeExp;
  const long long cache_size  = pow(2.0,sizeExp);
  webcache->setSize(cache_size);

  // parse cache parameters
  regex opexp ("(.*)=(.*)");
  cmatch opmatch;
  string paramSummary;
  for(int i=5; i<argc; i++) {
    regex_match (argv[i],opmatch,opexp);
    if(opmatch.size()!=3) {
      cerr << "each cacheParam needs to be in form name=value" << endl;
      return 1;
    }
    webcache->setPar(opmatch[1], opmatch[2]);
    paramSummary += opmatch[2];
  }

  ifstream infile;
  long long reqs = 0, bytes = 0;
  long long t, id, size;
  bool logStatistics=false;

  cerr << "running..." << endl;

  infile.open(path);
  std::chrono::duration<double> diff;
  auto  start=std::chrono::high_resolution_clock::now();
  while (infile >> t >> id >> size)
    {
      // start statistics after warm up
      if (!logStatistics && t > warmUp)
	{
	  cerr << "gathering statistics..." << endl;
	  logStatistics = true;
	  webcache->startStatistics();
	}

      // log statistics
      if (logStatistics)
	{
	  reqs++;
	  bytes += size;
	}

      // request
   // auto  start=std::chrono::high_resolution_clock::now();
      webcache->request(id,size);
 //   auto  end=std::chrono::high_resolution_clock::now();
   //  diff= end- start;
   // cout <<"diff"<<diff.count()<<endl;
    }
   auto  end=std::chrono::high_resolution_clock::now();
 diff =end -start;
cout <<"diff "<<(double)diff.count()<<endl;
 // std::chrono::duration<double> diff=end -start;
  infile.close();
  cout << "done." << endl << "-------" << endl
       << "cache policy: " << cacheType << endl
       << "size (log2): " << sizeExp << endl
       << "additional parameters: " << paramSummary << endl
       << "requests processed: " << reqs << endl
       << "object hit ratio: " << double(webcache->getHits())/reqs << endl
       << "byte hit ratio: " << double(webcache->getBytehits())/bytes << endl
       << "current cache size"<<webcache->getCurrentSize()<< endl
       << "Throughput::"<<double(reqs/diff.count())<<"s\n" ;

  return 0;
}
