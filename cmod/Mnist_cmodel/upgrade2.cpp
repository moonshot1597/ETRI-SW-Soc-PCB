#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

using namespace cv;
using namespace std;

//fixed line
typedef short FIXEDP;

inline FIXEDP toFixedp(double n)
{
	return (FIXEDP)(n*256);
}

inline FIXEDP fixedp_mul(FIXEDP a, FIXEDP b)
{
	return a * b >> 8;
}
inline FIXEDP fixedp_div(FIXEDP a, FIXEDP b)
{
	return ((long long)a * b) / 8;
}

//정렬
void sort_box(double *a, int size)
{
	int i, j, temp;
	
	for(i=size; i>0; i--){
		for(j=0; j<i-1; j++){	
			if(*(a+j)>*(a+j+1)){
			temp = *(a+j);
			*(a+j) = *(a+j+1); 
			*(a+j+1) = temp;
			}
		} 
	}
}

//full connect
void full_connect(int input_size, int output_size, double *input, double **weight, double *output, double *bias)
{
	int i, j;

	for(i=0; i<output_size; i++){
		for(j=0; j<input_size; j++){
			output[i] += weight[i][j] * input[j];
		}
	output[i] +=  bias[i];
	}
}

//Relu activation
double ReLu_act(double *input, int input_size)
{
	int i;

	for(i=0; i<input_size; i++)
		if(input[i]<0) input[i] = 0;
		else input[i] = input[i];
}

//Softmax activation
double Softmax_act(double *input, int input_size)
{
	double max = input[0];
	double sum;
	int i;

	for(i=0; i<input_size; i++){
		if(max<input[i])
		{		
			max = input[i];
		}
	}
	
	for(i=0; i<input_size; i++){
		input[i] = exp(input[i] - max);
		sum = sum + input[i];
	}

	for(i=0; i<input_size; i++){
		input[i] = input[i]/sum;
	} 
}



int main()
{
	//변수 선언
	int i, j, k, l, m, n, o, p;
	double max, sum;
	
	//크기 관련
	int stride;
	int channel_size, out_rows_size, out_cols_size;
	int num, rows_size, cols_size;
	int fch_size, f_size;
	int input_size, output_size, size, out_size;

	//배열들
	double ***input;
	double ***conv1_output;
	double ***pool1_output;
	double ***conv2_d_output;
	double ***conv2_p_output;
	double ***pool2_output;
	double *change_output;
	double *full1_output;
	double *full2_output;

	//웨이트 값들
	double ****conv1_weight;
	double ****conv2_d_weight;
	double ****conv2_p_weight;
	double **full_weight3;
	double **full_weight4;
	double *bias_weight1;
	double *bias_weight2;
	double *bias_weight3;
	double *bias_weight4;
	double *bias_weight5;

	//파일 읽기
	char ki;
	
	FILE *fp1;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	FILE *fp5;
	FILE *fp6;
	FILE *fp7;
	FILE *fp8;
	FILE *fp9;
	FILE *fp10;

	fp1 = fopen("./conv1.txt","r");
	fp2 = fopen("./conv2_dwise.txt","r");
	fp3 = fopen("./conv2_dwise_expand.txt","r");
	fp4 = fopen("./ip1.txt","r");
	fp5 = fopen("./ip2.txt","r");
	fp6 = fopen("./conv1_bias.txt","r");
	fp7 = fopen("./conv2_depth_bias.txt","r");
	fp8 = fopen("./conv2_point_bias.txt","r");
	fp9 = fopen("./ip1_bias.txt","r");
	fp10 = fopen("./ip2_bias.txt","r");

	//동적 할당
	//데이터 값들
	//1. input malloc
	channel_size = 1;
	rows_size = 28;
	cols_size = 28;

	input = (double ***)malloc(channel_size*sizeof(double**));

	for(i=0; i<channel_size; i++){
		*(input+i) = (double**)malloc(rows_size*sizeof(double*));
		for(j=0; j<rows_size; j++){
			*(*(input+i)+j) = (double*)malloc(cols_size*sizeof(double));
		}
	}
	
	//2. conv1_output malloc
	channel_size = 20;
	rows_size = 24;
	cols_size = 24;

	conv1_output = (double ***)malloc(channel_size*sizeof(double**));

	for(i=0; i<channel_size; i++){
		*(conv1_output+i) = (double**)malloc(rows_size*sizeof(double*));
		for(j=0; j<rows_size; j++){
			*(*(conv1_output+i)+j) = (double*)malloc(cols_size*sizeof(double));
		}
	}



	//3. pool1_output malloc
	channel_size = 20;
	rows_size = 12;
	cols_size = 12;

	pool1_output = (double ***)malloc(channel_size*sizeof(double**));

	for(i=0; i<channel_size; i++){
		*(pool1_output+i) = (double**)malloc(rows_size*sizeof(double*));
		for(j=0; j<rows_size; j++){
			*(*(pool1_output+i)+j) = (double*)malloc(cols_size*sizeof(double));
		}
	}

	//4-1. conv2_d_output malloc
	channel_size = 20;
	rows_size = 8;
	cols_size = 8;

	conv2_d_output = (double ***)malloc(channel_size*sizeof(double**));

	for(i=0; i<channel_size; i++){
		*(conv2_d_output+i) = (double**)malloc(rows_size*sizeof(double*));
		for(j=0; j<rows_size; j++){
			*(*(conv2_d_output+i)+j) = (double*)malloc(cols_size*sizeof(double));
		}
	}

	//4-2. conv2_p_output malloc
	channel_size = 50;
	rows_size = 8;
	cols_size = 8;

	conv2_p_output = (double ***)malloc(channel_size*sizeof(double**));

	for(i=0; i<channel_size; i++){
		*(conv2_p_output+i) = (double**)malloc(rows_size*sizeof(double*));
		for(j=0; j<rows_size; j++){
			*(*(conv2_p_output+i)+j) = (double*)malloc(cols_size*sizeof(double));
		}
	}


	//5. pool2_output malloc
	channel_size = 50;
	rows_size = 4;
	cols_size = 4;
	pool2_output = (double ***)malloc(channel_size*sizeof(double**));

	for(i=0; i<channel_size; i++){
		*(pool2_output+i) = (double**)malloc(rows_size*sizeof(double*));
		for(j=0; j<rows_size; j++){
			*(*(pool2_output+i)+j) = (double*)malloc(cols_size*sizeof(double));
		}
	}

	//6. change_output malloc
	size = 800;
	change_output = (double *)malloc(size*sizeof(double));

	//7. full1_output malloc
	size = 500;
	full1_output = (double *)malloc(size*sizeof(double));

	//8. full2_output malloc
	size = 10;
	full2_output = (double *)malloc(size*sizeof(double));

	//웨이트 값들
	//9. conv1_weight malloc
	num = 20;
	channel_size = 1;
	f_size = 5;
	
	conv1_weight = (double ****)malloc(num*sizeof(double***));

	for(i=0; i<num; i++){
		*(conv1_weight+i) = (double***)malloc(channel_size*sizeof(double**));
		for(j=0; j<channel_size; j++){
			*(*(conv1_weight+i)+j) = (double**)malloc(f_size*sizeof(double*));
			for(k=0; k<f_size; k++){
				*(*(*(conv1_weight+i)+j)+k) = (double*)malloc(f_size*sizeof(double));
			}
		}
	}

	//10-1. conv2_d_weight malloc
	num = 20;
	channel_size = 1;
	f_size = 5;
	
	conv2_d_weight = (double ****)malloc(num*sizeof(double***));

	for(i=0; i<num; i++){
		*(conv2_d_weight+i) = (double***)malloc(channel_size*sizeof(double**));
		for(j=0; j<channel_size; j++){
			*(*(conv2_d_weight+i)+j) = (double**)malloc(f_size*sizeof(double*));
			for(k=0; k<f_size; k++){
				*(*(*(conv2_d_weight+i)+j)+k) = (double*)malloc(f_size*sizeof(double));
			}
		}
	}

	//10-2. conv2_p_weight malloc
	num = 50;
	channel_size = 20;
	f_size = 1;
	
	conv2_p_weight = (double ****)malloc(num*sizeof(double***));

	for(i=0; i<num; i++){
		*(conv2_p_weight+i) = (double***)malloc(channel_size*sizeof(double**));
		for(j=0; j<channel_size; j++){
			*(*(conv2_p_weight+i)+j) = (double**)malloc(f_size*sizeof(double*));
			for(k=0; k<f_size; k++){
				*(*(*(conv2_p_weight+i)+j)+k) = (double*)malloc(f_size*sizeof(double));
			}
		}
	}

	//11. full_weight3 malloc
	output_size = 500;
	input_size = 800;

	full_weight3 = (double **)malloc(output_size*sizeof(double*));
	for(i=0; i<output_size; i++)
	{
		*(full_weight3 + i) = (double *)malloc(input_size*sizeof(double));
	}		

	//12. full_weight4 malloc
	output_size = 10;
	input_size = 500;

	full_weight4 = (double **)malloc(output_size*sizeof(double*));
	for(i=0; i<output_size; i++)
	{
		*(full_weight4 + i) = (double *)malloc(input_size*sizeof(double));
	}


	//13. bias_weight1 malloc
	size = 20;
	bias_weight1 = (double *)malloc(size*sizeof(double));

	//14. bias_weight2 malloc
	size = 20;
	bias_weight2 = (double *)malloc(size*sizeof(double));

	//15. bias_weight3 malloc
	size = 50;
	bias_weight3 = (double *)malloc(size*sizeof(double));

	//16. bias_weight4 malloc
	size = 500;
	bias_weight4 = (double *)malloc(size*sizeof(double));

	//17. bias_weight5 malloc
	size = 10;
	bias_weight5 = (double *)malloc(size*sizeof(double));



	//웨이트 값 받기
	//conv1_weight
	channel_size = 20;
	fch_size = 1;
	f_size = 5;

	for(i=0; i<channel_size; i++){
		for(j=0; j<fch_size; j++){
			for(k=0; k<f_size; k++){
				for(l=0; l<f_size; l++){
					fscanf(fp1, " %lf ,", (*(*(*(conv1_weight+i)+j)+k)+l));					
				}
			}
		}
	}

	//conv2_d_weight
	channel_size = 20;
	fch_size = 1;
	f_size = 5;

	for(i=0; i<channel_size; i++){
		for(j=0; j<fch_size; j++){
			for(k=0; k<f_size; k++){
				for(l=0; l<f_size; l++){
				fscanf(fp2, " %lf ,", (*(*(*(conv2_d_weight+i)+j)+k)+l));					
				}
			}
		}
	}

	//conv2_p_weight
	channel_size = 50;
	fch_size = 20;
	f_size = 1;

	for(i=0; i<channel_size; i++){
		for(j=0; j<fch_size; j++){
			for(k=0; k<f_size; k++){
				for(l=0; l<f_size; l++){
				fscanf(fp3, " %lf ,", (*(*(*(conv2_p_weight+i)+j)+k)+l));					
				}
			}
		}
	}


	//full_weight3
	output_size = 500;
	input_size = 800;

	for(i=0; i<output_size; i++){
		for(j=0; j<input_size; j++){
			fscanf(fp4, " %lf ,", (*(full_weight3+i)+j));					
		}
	}

	
	//full_weight4
	output_size = 10;
	input_size = 500;

	for(i=0; i<output_size; i++){
		for(j=0; j<input_size; j++){
			fscanf(fp5, " %lf ,", (*(full_weight4+i)+j));					
		}
	}

	//bias_weight1
	size = 20;

	for(i=0; i<size; i++){
		fscanf(fp6, " %lf ,", (bias_weight1+i));
	}




	//bias_weight2
	size = 20;

	for(i=0; i<size; i++){
		fscanf(fp7, " %lf ,", (bias_weight2+i));
	}



	//bias_weight3
	size = 50;

	for(i=0; i<size; i++){
		fscanf(fp8, " %lf ,", (bias_weight3+i));
	}


	//bias_weight4
	size = 500;

	for(i=0; i<size; i++){
		fscanf(fp9, " %lf ,", (bias_weight4+i));
	}

	//bias_weight5
	size = 10;

	for(i=0; i<size; i++){
		fscanf(fp10, " %lf ,", (bias_weight5+i));
	}



	//이미지 받기
	Mat image;
	image = imread("0.png",IMREAD_GRAYSCALE);
	if(image.empty())
	{
		cout<<"Could not open or find the image" <<endl;
		return -1;
	}


	

	//input<<image //mat 2차원 포인터 확인하기 //상관없음 확인
	channel_size = 1;
	for(i=0; i<channel_size; i++){
		for(j=0; j<image.rows; j++){
			for(k=0; k<image.cols; k++){
				*(*(*(input+i)+j)+k) = image.at<uchar>(j,k);
			}
		}
	}
	

	//conv1
	channel_size = 20;
	out_rows_size = 24;
	out_cols_size = 24;
	fch_size = 1;
	f_size = 5;
	stride = 1;

	for(i=0; i<channel_size; i++){
		for(j=0; j<out_rows_size; j++){
			for(k=0; k<out_cols_size; k++){
			*(*(*(conv1_output+i)+j)+k) += *(bias_weight1+i);
				for(m=0; m<fch_size; m++){
					for(n=0; n<f_size; n++){
						for(o=0; o<f_size; o++){
							//sum += (*(*(*(input+m)+(j*stride+n))+(k*stride+o)) * *(*(*(*(conv1_weight+i)+m)+n)+o));
							*(*(*(conv1_output+i)+j)+k) += *(*(*(input+m)+(j*stride+n))+(k*stride+o)) * *(*(*(*(conv1_weight+i)+m)+n)+o);
						}			
					}
				}
			}
		}
	}



	//pool1
	channel_size = 20;
	out_rows_size = 12;
	out_cols_size = 12;
	fch_size = 1;
	f_size = 2;
	stride = 2;

	for(i=0; i<channel_size; i++){
		for(j=0; j<out_rows_size; j++){
			for(k=0; k<out_cols_size; k++){
				//if(m!=fch_size-1)max = *(*(*(conv1_output+i)+(j*stride+n))+(k*stride+o));
				max = INT_MIN;
				for(m=0; m<fch_size; m++){
					for(n=0; n<f_size; n++){
						for(o=0; o<f_size; o++){
								if(max<*(*(*(conv1_output+i)+(j*stride+n))+(k*stride+o)))
								max = *(*(*(conv1_output+i)+(j*stride+n))+(k*stride+o));
						}
					}
				*(*(*(pool1_output+i)+j)+k) = max;
				}
			}
		}
	}

	//conv2_dwise
	channel_size = 20;
	out_rows_size = 8;
	out_cols_size = 8;
	fch_size = 1;
	f_size = 5;
	stride = 1;

	for(i=0; i<channel_size; i++){
		for(j=0; j<out_rows_size; j++){
			for(k=0; k<out_cols_size; k++){
			*(*(*(conv2_d_output+i)+j)+k) += *(bias_weight2+i);
				for(m=0; m<fch_size; m++){
					for(n=0; n<f_size; n++){
						for(o=0; o<f_size; o++){
							//sum += (*(*(*(pool1_output+m)+(j*stride+n))+(k*stride+o)) * *(*(*(*(conv2_weight+i)+m)+n)+o));
							*(*(*(conv2_d_output+i)+j)+k) += *(*(*(pool1_output+i)+(j*stride+n))+(k*stride+o)) * *(*(*(*(conv2_d_weight+i)+m)+n)+o);
						}
					}
				}
			}		
		}
	}

	//conv2_point
	channel_size = 50;
	out_rows_size = 8;
	out_cols_size = 8;
	fch_size = 20;
	f_size = 1;
	stride = 1;

	for(i=0; i<channel_size; i++){
		for(j=0; j<out_rows_size; j++){
			for(k=0; k<out_cols_size; k++){
			*(*(*(conv2_p_output+i)+j)+k) += *(bias_weight3+i);
				for(m=0; m<fch_size; m++){
					for(n=0; n<f_size; n++){
						for(o=0; o<f_size; o++){
							//sum += (*(*(*(pool1_output+m)+(j*stride+n))+(k*stride+o)) * *(*(*(*(conv2_weight+i)+m)+n)+o));
							*(*(*(conv2_p_output+i)+j)+k) += *(*(*(conv2_d_output+m)+(j*stride+n))+(k*stride+o)) * *(*(*(*(conv2_p_weight+i)+m)+n)+o);
						}
					}
				}
			}		
		}
	}




	//pool2
	channel_size = 50;
	out_rows_size = 4;
	out_cols_size = 4;
	fch_size = 1;
	f_size = 2;
	stride = 2;

	for(i=0; i<channel_size; i++){
		for(j=0; j<out_rows_size; j++){
			for(k=0; k<out_cols_size; k++){
				//if(m!=fch_size-1)max = *(*(*(conv2_output+i)+(j*stride+n))+(k*stride+o));
				max = INT_MIN;
				for(m=0; m<fch_size; m++){
					for(n=0; n<f_size; n++){
						for(o=0; o<f_size; o++){
								if(max<*(*(*(conv2_p_output+i)+(j*stride+n))+(k*stride+o)))
								max = *(*(*(conv2_p_output+i)+(j*stride+n))+(k*stride+o));
							}
						}
						*(*(*(pool2_output+i)+j)+k) = max;
				}
			
			}
		}
	}



	
	


	//3d to 1d
	channel_size = 50;
	rows_size = 4;
	cols_size = 4;

	for(i=0; i<channel_size; i++){
		for(j=0; j<rows_size; j++){
			for(k = 0; k<cols_size; k++){
				*(change_output+i*rows_size*cols_size+j*rows_size+k) = *(*(*(pool2_output+i)+j)+k);
			}
		}
	}


	//fc
	//full connect1
	input_size = 800;
	output_size = 500;

	full_connect(input_size, output_size, change_output, full_weight3, full1_output, bias_weight4);
		

	//Relu activation
	input_size = 500;
	ReLu_act(full1_output, input_size);


	//full connect2
	input_size = 500;
	output_size = 10;

	full_connect(input_size, output_size, full1_output, full_weight4, full2_output, bias_weight5);

	input_size = 10;

	for(i=0; i<input_size; i++){
		printf("[%d] %.20lf \n", i , *(full2_output+i));
	}
	printf("\n");

	
	//Softmax activation
	input_size = 10;

	Softmax_act(full2_output, input_size);





	//result
	for(i=0; i<input_size; i++){
		printf("[%d] %.20lf \n", i , *(full2_output+i));
	}
	printf("\n");

	//free해주기
	//데이터 값들
	//1.input free
	channel_size = 1;
	rows_size = 28;

	for(i=0; i<channel_size; i++){
		 for(j=0; j<rows_size; j++){
			free(*(*(input+i)+j));
		}
		free(*(input+i));
	}
	free(input);

	//2. conv1_output free
	channel_size = 20;
	rows_size = 24;

	for(i=0; i<channel_size; i++){
		 for(j=0; j<rows_size; j++){
			free(*(*(conv1_output+i)+j));
		}
		free(*(conv1_output+i));
	}
	free(conv1_output);

	//3. pool1_output free
	channel_size = 20;
	rows_size = 12;


	for(i=0; i<channel_size; i++){
		 for(j=0; j<rows_size; j++){
			free(*(*(pool1_output+i)+j));
		}
		free(*(pool1_output+i));
	}
	free(pool1_output);

	//4-1. conv2_d_output free
	channel_size = 20;
	rows_size = 8;

	for(i=0; i<channel_size; i++){
		 for(j=0; j<rows_size; j++){
			free(*(*(conv2_d_output+i)+j));
		}
		free(*(conv2_d_output+i));
	}
	free(conv2_d_output);

	//4-2. conv2_p_output free
	channel_size = 50;
	rows_size = 8;

	for(i=0; i<channel_size; i++){
		 for(j=0; j<rows_size; j++){
			free(*(*(conv2_p_output+i)+j));
		}
		free(*(conv2_p_output+i));
	}
	free(conv2_p_output);

	//5. pool2_output free
	channel_size = 50;
	rows_size = 4;

	for(i=0; i<channel_size; i++){
		 for(j=0; j<rows_size; j++){
			free(*(*(pool2_output+i)+j));
		}
		free(*(pool2_output+i));
	}
	free(pool2_output);

	//6. change_output free
	free(change_output);

	//7. full1_output free
	free(full1_output);

	//8. full2_output free
	free(full2_output);

	//웨이트 값들
	//9. conv1_weight free
	num = 20;
	channel_size = 1;
	rows_size = 5;
	
	for(i=0; i<num; i++){
		 for(j=0; j<channel_size; j++){
			for(k=0; k<rows_size; k++){
				free(*(*(*(conv1_weight+i)+j)+k));		
			}
			free(*(*(conv1_weight+i)+j));
		}
		free(*(conv1_weight+i));
	}
	free(conv1_weight);

	//10-1. conv2_d_weight free
	num = 20;
	channel_size = 1;
	rows_size = 5;

	for(i=0; i<num; i++){
		 for(j=0; j<channel_size; j++){
			for(k=0; k<rows_size; k++){
				free(*(*(*(conv2_d_weight+i)+j)+k));		
			}
			free(*(*(conv2_d_weight+i)+j));
		}
		free(*(conv2_d_weight+i));
	}
	free(conv2_d_weight);

	//10-2. conv2_p_weight free
	num = 50;
	channel_size = 20;
	rows_size = 1;

	for(i=0; i<num; i++){
		 for(j=0; j<channel_size; j++){
			for(k=0; k<rows_size; k++){
				free(*(*(*(conv2_p_weight+i)+j)+k));		
			}
			free(*(*(conv2_p_weight+i)+j));
		}
		free(*(conv2_p_weight+i));
	}
	free(conv2_p_weight);

	//11. full_weight3 free
	out_size = 500;

	for(i=0; i<out_size; i++)free(*(full_weight3+i));
	free(full_weight3);

	//12. full_weight4 free
	out_size = 10;

	for(i=0; i<out_size; i++)free(*(full_weight4+i));
	free(full_weight4);

	//13. bias_weight1 free
	free(bias_weight1);

	//14. bias_weight2 free
	free(bias_weight2);

	//15. bias_weight3 free
	free(bias_weight3);

	//16. bias_weight4 free
	free(bias_weight4);

	//17. bias_weight4 free
	free(bias_weight5);



	return 0;
}
