#include "./helper.h"
#include "algo.h"

BOOST_AUTO_TEST_CASE(trace_back_test)
{
  //initilization of input
  string str1 = "apple";
  string str2 = "saturday";
  int32_t dist[str1.length()+1][str2.length()+1];
  for(int32_t i=0;i<str1.length()+1;i++){
      for(int32_t j=0;j<str2.length()+1;j++){
          if(i==0) dist[i][j]=j;
          else if(j==0) dist[i][j] = i;
          else if (str1[i-1]==str2[j-1]){
              dist[i][j] = dist[i-1][j-1];
          }
          else{
            if(dist[i-1][j-1] <= dist[i][j-1]&&dist[i-1][j-1] <= dist[i-1][j]){
                dist[i][j] = dist[i-1][j-1]+1;
            }
            else if(dist[i][j-1] <= dist[i-1][j-1]&&dist[i][j-1] <= dist[i-1][j]){
                dist[i][j] = dist[i][j-1]+1;
            }
            else if(dist[i-1][j] <= dist[i-1][j-1]&&dist[i-1][j] <= dist[i][j-1]){
                dist[i][j] = dist[i-1][j]+1;
            }
          }
      }
  }
  int32_t input[(str1.length()+1)*(str2.length()+1)];

  for(int32_t k=0;k<str1.length()+1;k++){
      cout<<"\n";
    for(int32_t l=0;l<str2.length()+1;l++){
        input[l+k*(str2.length()+1)] = dist[k][l];
        cout<<input[l+k*(str2.length()+1)]<<" ";
      }
  }
  //initializatoin of output
  cout<<"\n";
  int32_t output[(str1.length()+1)*(str2.length()+1)];
  for(int k=0;k<(str1.length()+1)*(str2.length()+1);k++){
        output[k] =0;
  }
  output[str1.length()*(str2.length()+1)+str2.length()] = dist[str1.length()][str2.length()];
  //initialization of length
  int32_t len = (str1.length()+1)*(str2.length()+1);
  int32_t col = str2.length()+1;

  trace_back(input, output, len, col,str1.length(),str2.length());
  for(int k=0;k<(str1.length()+1)*(str2.length()+1);k++){
  	if(k%(str2.length()+1)==0) cout<<"\n";
        cout<<output[k]<<" ";
    }
cout<<"\n"<<"\n";
cout<<"running time is "<< clock();   	
}
