// LH.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <iomanip>
#include <cstring>


using namespace std;
int time_num; //时间
int customer_num;//客户数量
int site_num; //节点数量
int QOS_CONSTRAINT;
int allow_data[36][136];
int QOS_data[36][136];
int SB_data[136];
int SB_data_mean;
int DEMAND_data[8929][136];
string customers_name[36];
string sites_name[136];


string DEMAND_PATH = "/data/demand.csv";
string QOS_PATH = "/data/qos.csv";
string SITE_BANDWIDTH_PATH = "/data/site_bandwidth.csv";
string configPath = "/data/config.ini";
char output_path[100] = "/output/solution.txt";


//string DEMAND_PATH = "C:\\Users\\LHJ\\Desktop\\华为比赛\\code2\\data\\demand.csv";
//string QOS_PATH = "C:\\Users\\LHJ\\Desktop\\华为比赛\\code2\\data\\qos.csv";
//string SITE_BANDWIDTH_PATH = "C:\\Users\\LHJ\\Desktop\\华为比赛\\code2\\data\\site_bandwidth.csv";
//string configPath = "C:\\Users\\LHJ\\Desktop\\华为比赛\\code2\\data\\config.ini";
//char output_path[300] = "C:\\Users\\LHJ\\Desktop\\华为比赛\\code2\\data\\solution.txt";

//string DEMAND_PATH = "/home/jasen/data/demand.csv";
//string QOS_PATH = "/home/jasen/data/qos.csv";
//string SITE_BANDWIDTH_PATH = "/home/jasen/data/site_bandwidth.csv";
//string configPath = "/home/jasen/data/config.ini";
//char output_path[100] = "solution.txt";


int max(int a[], int n)
{
	float max = a[0];
	for (int i = 0; i < n; i++)
	{
		if (a[i] > max)
			max = a[i];
	}
	return max;
}
int max_index(int a[], int n)
{
	int max = a[0];
	int index = 0;
	for (int i = 0; i < n; i++)
	{
		if (a[i] > max)
		{
			max = a[i];
			index = i;
		}

	}
	return index;
}
int mean(int a[], int n)
{
	int sum = a[0];
	for (int i = 1; i < n; i++)
	{
		sum = sum + a[i];
	}
	return sum / n;
}
float max(float a[], int n)
{
	float max = a[0];
	for (int i = 0; i < n; i++)
	{
		if (a[i] > max)
			max = a[i];
	}
	return max;
}

int min(int b[], int n)
{
	float min = b[0];
	for (int i = 0; i < n; i++)
	{
		if (b[i] < min)
			min = b[i];
	}
	return min;

}

void clc(int b[], int n) //矩阵清零
{
	for (int i = 0; i < n; i++)
	{
		b[i] = 0;
	}
}

void clc(float b[], int n) //矩阵清零
{
	for (int i = 0; i < n; i++)
	{
		b[i] = 0.0;
	}
}


string Trim(string& str)
{
	//str.find_first_not_of(" \t\r\n"),在字符串str中从索引0开始，返回首次不匹配"\t\r\n"的位置
	str.erase(0, str.find_first_not_of(" \t\r\n"));
	str.erase(str.find_last_not_of(" \t\r\n") + 1);
	return str;
}
int dropout_site_index[135];
int dropout_site_num;
int Qos_customer_index[35];
int Qos_site_index[135];

void Read_data()
{
	ifstream fin(DEMAND_PATH); //打开文件流操作
	string line;
	int j = 0;
	int i = 0;
	while (getline(fin, line))   //整行读取，换行符“\n”区分，遇到文件尾标志eof终止读取
	{
		istringstream sin(line); //将整行字符串line读入到字符串流istringstream中
		vector<string> fields; //声明一个字符串向量
		string field;
		while (getline(sin, field, ',')) //将字符串流sin中的字符读入到field字符串中，以逗号为分隔符
		{
			fields.push_back(field); //将刚刚读取的字符串添加到向量fields中
		}

		i = 0;
		for (auto lin : fields) {
			if ((i != 0) && (j == 0))
			{
				customers_name[i - 1] = Trim(lin);
			}
			if ((i != 0) && (j != 0))
			{
				DEMAND_data[j - 1][i - 1] = atoi(lin.c_str());
			}
			i = i + 1;
		}
		j = j + 1;
	}
	time_num = j - 1;
	customer_num = i - 1;
	if ((customer_num < 1) || (time_num < 1))
	{
		exit(1);
	}
	ifstream fin3(SITE_BANDWIDTH_PATH); //打开文件流操作
	j = 0;
	while (getline(fin3, line))   //整行读取，换行符“\n”区分，遇到文件尾标志eof终止读取
	{
		istringstream sin3(line);
		vector<string> fields3;
		string field3;
		while (getline(sin3, field3, ',')) //将字符串流sin中的字符读入到field字符串中，以逗号为分隔符
		{
			fields3.push_back(field3); //将刚刚读取的字符串添加到向量fields中
		}
		i = 0;

		for (auto lin : fields3) {
			if ((i == 0) && (j != 0))
			{
				sites_name[j - 1] = Trim(lin);
			}
			if ((i != 0) && (j != 0))
			{
				SB_data[j - 1] = atoi(lin.c_str());
			}
			i = i + 1;
		}
		j = j + 1;
	}

	site_num = j - 1;
	if (site_num < 1)
	{
		exit(1);
	}
	int SB_data_mean;
	SB_data_mean = mean(SB_data, site_num);

	/*int index = 0;
	for (i = 0; i < site_num; i++)
	{
		if (SB_data[i] < SB_data_mean * 0.1)
		{
			dropout_site_index[index] = i;
			index = index + 1;
			dropout_site_num = index;
		}

	}*/
	ifstream fin2(QOS_PATH); //打开文件流操作
	j = 0;
	int customer;
	int qos_customer_index;
	int qos_site_index;
	int site;
	string Qos_cusstomer_name;
	string Qos_site_name;
	qos_customer_index = 0;
	qos_site_index = 0;
	while (getline(fin2, line))   //整行读取，换行符“\n”区分，遇到文件尾标志eof终止读取
	{
		istringstream sin2(line);
		vector<string> fields2;
		string field2;
		while (getline(sin2, field2, ',')) //将字符串流sin中的字符读入到field字符串中，以逗号为分隔符
		{
			fields2.push_back(field2); //将刚刚读取的字符串添加到向量fields中
		}
		i = 0;

		for (auto lin : fields2) {
			if ((j == 0) && (i != 0))
			{
				Qos_cusstomer_name = Trim(lin);
				for (customer = 0; customer < customer_num; customer++)
				{
					if (Qos_cusstomer_name == customers_name[customer])
					{
						Qos_customer_index[qos_customer_index] = customer;
						qos_customer_index = qos_customer_index + 1;
					}
				}

			}
			if ((i == 0) && (j != 0))
			{
				Qos_site_name = Trim(lin);
				for (site = 0; site < site_num; site++)
				{
					if (Qos_site_name == sites_name[site])
					{
						Qos_site_index[qos_site_index] = site;
						qos_site_index = qos_site_index + 1;
					}
				}
			}
			if ((i != 0) && (j != 0))
			{
				QOS_data[Qos_customer_index[i - 1]][Qos_site_index[j - 1]] = atoi(lin.c_str());
			}
			i = i + 1;
		}
		j = j + 1;
	}

	ifstream fin4(configPath);
	j = 0;
	while (getline(fin4, line))   //整行读取，换行符“\n”区分，遇到文件尾标志eof终止读取
	{
		istringstream sin4(line);
		vector<string> fields4;
		string field4;
		while (getline(sin4, field4, '=')) //将字符串流sin中的字符读入到field字符串中，以逗号为分隔符
		{
			fields4.push_back(field4); //将刚刚读取的字符串添加到向量fields中
		}
		i = 0;
		int position = 0;
		for (auto lin : fields4)
		{
			if (position == 1)
			{
				QOS_CONSTRAINT = atoi(lin.c_str());
			}
			if (strcmp(lin.c_str(), "qos_constraint") == 0)
			{
				position = 1;
			}
			i = i + 1;
		}
		j = j + 1;
	}
	if ((QOS_CONSTRAINT < 100) && (QOS_CONSTRAINT > 400))
	{
		exit(1);//读qos的值失败了
	}

	int z;
	int drop_out;
	for (j = 0; j < site_num; j++)
	{

		for (i = 0; i < customer_num; i++)
		{
			if ((QOS_data[i][j] < QOS_CONSTRAINT))
			{
				allow_data[i][j] = 1;
			}
			else
			{
				allow_data[i][j] = 0;
			}

		}

	}

}
int real_site_index[135];
int real_site_num = 0;
int real_allow_data[35][135];
int allow_data_clo_sum[135]; //行和，如果为0则代表节点完全不可用

void site_fifter()//求出实际能用的site的索引
{
	int i, j, line_sum, index;
	index = 0;
	for (j = 0; j < site_num; j++)
	{
		line_sum = 0;
		for (i = 0; i < customer_num; i++)
		{
			line_sum = line_sum + allow_data[i][j];
		}
		if (line_sum != 0)
		{
			real_site_index[index] = j;
			allow_data_clo_sum[index] = line_sum;  //列和
			index = index + 1;
		}

	}
	real_site_num = index;

	for (j = 0; j < real_site_num; j++)
	{
		for (i = 0; i < customer_num; i++)
		{
			real_allow_data[i][j] = allow_data[i][real_site_index[j]];
		}
	}


}


int slove_Mat_all[8929][36][136]; //全部的解存放矩阵
int slove_one_Mat[36][136];
int site_power_mat[136];
int denominator;
int avg, less_band;
int use_site_num;
int allow_site_index[136];
float gravitation_list[136];
float gravitation_list_max, gravitation_list_min;
int slove_one_Mat_clo_sum[136];



void calc_gravitation_list(int T, int customer) //计算重力矩阵
{
	//求解的列和
	int i, j;
	clc(gravitation_list, real_site_num);
	gravitation_list_max = 0;
	gravitation_list_min = 0;
	for (j = 0; j < real_site_num; j++)
	{
		slove_one_Mat_clo_sum[j] = 0;
		for (i = 0; i < customer_num; i++)
		{
			slove_one_Mat_clo_sum[j] = slove_one_Mat_clo_sum[j] + slove_Mat_all[T][i][j];
		}
	}
	gravitation_list_max = max(slove_one_Mat_clo_sum, real_site_num);
	gravitation_list_min = min(slove_one_Mat_clo_sum, real_site_num);
	for (j = 0; j < real_site_num; j++)
	{
		if (slove_one_Mat_clo_sum[j] >= SB_data[real_site_index[j]] * 0.95)
		{
			gravitation_list[j] = 5;
		}
		else
		{
			gravitation_list[j] = (slove_one_Mat_clo_sum[j] - gravitation_list_min) / (gravitation_list_max - gravitation_list_min);
		}
	}


}
int transport_list[136];
int max_moving_band[136];
float gravitational;
int transport_band;
int set_step = 2000;
int step_weekn = 1;
int move_band = 0;
int Target = 2000;
int max_calc = 3000;
int step = 0;
int stop;
int patience_max = 5;

void calc_transport_list(int T, int customer) //计算搬运矩阵
{
	int i, j;
	clc(transport_list, real_site_num);//清零
	clc(max_moving_band, real_site_num);
	for (i = 0; i < real_site_num; i++)
	{
		max_moving_band[i] = SB_data[real_site_index[i]] * 0.95 - slove_one_Mat_clo_sum[i];
		if (max_moving_band[i] <= 0)
		{
			max_moving_band[i] = 0;
		}
	}
	for (i = 0; i < real_site_num; i++)
	{
		if (real_allow_data[customer][i] != 0)
		{
			for (j = 0; j < real_site_num; j++)
			{

				if ((real_allow_data[customer][j] != 0) && (i != j))
				{
					gravitational = -1 * (gravitation_list[i] - gravitation_list[j]);
					if (gravitational > 0.1)
					{
						gravitational = 0.1;
					}
					if (gravitational < -0.1)
					{
						gravitational = -0.1;
					}
					transport_band = (gravitational * step);
					if ((transport_band < 0) && (slove_Mat_all[T][customer][i] + transport_band + transport_list[i] <= 0))
					{
						// 如果ia搬走的太多，就不行，只能搬走还剩下的
						move_band = slove_Mat_all[T][customer][i] + transport_list[i];
						if (move_band < 0)
						{
							move_band = -1 * move_band;
						}
						transport_list[i] = transport_list[i] - move_band;
						transport_list[j] = transport_list[j] + move_band;
					}

					else if ((transport_band > 0) && (slove_Mat_all[T][customer][j] - transport_band + transport_list[j] <= 0))
					{
						//如果ma搬走了太多，也要只能搬走剩下了的
						move_band = slove_Mat_all[T][customer][j] + transport_list[j];
						if (move_band < 0)
						{
							move_band = -1 * move_band;
						}
						transport_list[i] = transport_list[i] + move_band;
						transport_list[j] = transport_list[j] - move_band;
					}
					else if ((transport_band < 0) && (-1 * transport_band + transport_list[j] >= max_moving_band[j]))

					{ // 也不能一进去太多，不然也会超上限
						move_band = max_moving_band[j] - transport_list[j];
						transport_list[i] = transport_list[i] - move_band;
						transport_list[j] = transport_list[j] + move_band;
					}
					else if ((transport_band > 0) && (transport_band + transport_list[i] >= max_moving_band[i]))

					{ // 也不能一进去太多，不然也会超上限
						move_band = max_moving_band[i] - transport_list[i];
						transport_list[i] = transport_list[i] + move_band;
						transport_list[j] = transport_list[j] - move_band;
					}
					else
					{
						transport_list[i] = transport_list[i] + transport_band;
						transport_list[j] = transport_list[j] - transport_band;
					}
				}

			}
		}
	}

}
void calc_init_slove(int T)
{
	int i, j;
	for (i = 0; i < real_site_num; i++)
	{
		site_power_mat[i] = customer_num - allow_data_clo_sum[i] + 1;
	}
	for (i = 0; i < customer_num; i++)
	{
		avg = 0;
		less_band = 0;
		if (DEMAND_data[T][i] != 0)
		{
			//求分权分母
			denominator = 0;
			use_site_num = 0;
			for (j = 0; j < real_site_num; j++)
			{
				if (real_allow_data[i][j] != 0)
				{
					denominator = denominator + site_power_mat[j];
					allow_site_index[use_site_num] = j;
					use_site_num = use_site_num + 1;
				}
			}
			avg = DEMAND_data[T][i] / denominator;
			less_band = DEMAND_data[T][i];

			for (j = 0; j < use_site_num - 1; j++)
			{
				slove_Mat_all[T][i][allow_site_index[j]] = avg * site_power_mat[allow_site_index[j]];
				less_band = less_band - (avg * site_power_mat[allow_site_index[j]]); //1用来补偿
			}
			slove_Mat_all[T][i][allow_site_index[use_site_num - 1]] = less_band;
		}
	}
}
int Performance, last_Performance;
int patience;
int check_slove_out(int T, int calc_num)  //如果不合法就不能输出
{
	int j, allow;
	allow = 0;
	int out = 0;
	for (j = 0; j < real_site_num; j++)
	{
		if (slove_one_Mat_clo_sum[j] >= SB_data[real_site_index[j]])
		{
			allow = 1;
		}
	}

	if (allow != 1)
	{
		Performance = max(slove_one_Mat_clo_sum, real_site_num) - min(slove_one_Mat_clo_sum, real_site_num);
		/*cout << " T:" << T << " calc_num:" << calc_num << " allow:" << allow << endl;
		cout << " Performance:" << Performance << endl;
		cout << " patience:" << patience << endl;*/


		if (calc_num < 2)
		{
			last_Performance = Performance;
		}
		if (Performance < Target)
		{
			out = 1;
		}
		if (last_Performance < Performance)
		{
			patience = patience + 1;
			if (patience > patience_max)
			{
				out = 1;
			}

		}
		if (last_Performance >= Performance)
		{
			patience = 0;
			last_Performance = Performance;
		}


		if (calc_num > max_calc)
		{
			out = 1;
		}


	}
	return out;

}

int slove_one_Mat_clo_sum_2[136];

void check_slove_out_2(int T, int calc_num)
{
	int customer_need[35];
	int j, allow;
	int customer;
	allow = 0;
	int i;
	clc(slove_one_Mat_clo_sum_2, real_site_num);
	clc(customer_need, customer_num);
	for (customer = 0; customer < customer_num; customer++)
	{
		for (i = 0; i < real_site_num; i++)
		{
			if (slove_Mat_all[T][customer][i] < 0)
			{
				allow = 4;
			}
		}
	}
	for (j = 0; j < real_site_num; j++)
	{
		slove_one_Mat_clo_sum[j] = 0;
		for (i = 0; i < customer_num; i++)
		{
			slove_one_Mat_clo_sum_2[j] = slove_one_Mat_clo_sum_2[j] + slove_Mat_all[T][i][j];
		}
	}
	for (j = 0; j < real_site_num; j++)
	{
		if (slove_one_Mat_clo_sum_2[j] >= SB_data[real_site_index[j]])
		{
			allow = 1;
		}
	}
	for (i = 0; i < customer_num; i++)
	{
		for (j = 0; j < real_site_num; j++)
		{
			if (real_allow_data[i][j] == 0)
			{
				if (slove_Mat_all[T][i][j] != 0)
				{
					allow = 2;
				}
			}
		}
	}

	for (i = 0; i < customer_num; i++)
	{
		for (j = 0; j < real_site_num; j++)
		{
			customer_need[i] = customer_need[i] + slove_Mat_all[T][i][j];
		}

		if (customer_need[i] != DEMAND_data[T][i])
		{
			allow = 3;
		}
	}
	if (allow == 1)
	{
		exit(0);  //异常退出是超带宽
	}
	if (allow == 2)
	{
		exit(0); //异常退出是qos错了
	}
	if (allow == 3)
	{
		exit(1); //异常退出是客户需求错了
	}
	if (allow == 4)
	{
		exit(1); //异常退出是为负了
	}
}
void slove_mat()
{
	int i, j, z, T, calc_num, this_time_step;
	int slover_right = 0;
	for (T = 0; T < time_num; T++)
	{
		j = 0;
		calc_num = 0;
		calc_init_slove(T);
		calc_gravitation_list(T, j);
		step = set_step;
		patience = 0;
		while (true)
		{
			if (check_slove_out(T, calc_num) == 1)
			{
				check_slove_out_2(T, calc_num);
				break;
			}
			calc_transport_list(T, j);
			for (z = 0; z < real_site_num; z++)
			{
				slove_Mat_all[T][j][z] = slove_Mat_all[T][j][z] + transport_list[z];
			}
			if (j == customer_num - 1)
			{
				j = 0;
			}
			else
			{
				j = j + 1;
			}
			calc_num = calc_num + 1;
			step = step - step_weekn;
			if (step < 10)
			{
				step = 10;
			}
			calc_gravitation_list(T, j);
		}

	}
}


int band_work[136][8929];
int band_work_sort_index[136][8929];

void BubbleSort(int* p, int length, int* ind_diff)
{
	for (int m = 0; m < length; m++)
	{
		ind_diff[m] = m;
	}

	for (int i = 0; i < length; i++)
	{
		for (int j = 0; j < length - i - 1; j++)
		{
			if (p[j] > p[j + 1])
			{
				int temp = p[j];
				p[j] = p[j + 1];
				p[j + 1] = temp;

				int ind_temp = ind_diff[j];
				ind_diff[j] = ind_diff[j + 1];
				ind_diff[j + 1] = ind_temp;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
int T_95_sum[8929]; //T时刻95的个数
int T_target[135]; //95所在时刻
int out_site_index;
int out_site_num;
int in_site_index;
int in_site_num;
int output_valuel;
int choose_time;
int out_site[135];//输出的结点
int in_site[135];
int getvalueT;
int max_clac_T = 3000;
int Last_output_valuel;
int patience_T = 0;
int patience_max_T = 30;
int step_T = 100;
int step_T2 = 50;

int check_slove_out_for_T(int T, int calc_num)  //如果不合法就不能输出
{
	int i,j, allow;
	allow = 0;
	int out = 0;
	for (j = 0; j < real_site_num; j++)
	{
		slove_one_Mat_clo_sum[j] = 0;
		for (i = 0; i < customer_num; i++)
		{
			slove_one_Mat_clo_sum[j] = slove_one_Mat_clo_sum[j] + slove_Mat_all[T][i][j];
		}
	}//要计算最大值


	for (j = 0; j < real_site_num; j++)
	{
		if (slove_one_Mat_clo_sum[j] >= SB_data[real_site_index[j]])
		{
			allow = 1;
			exit(0);
		}
	}

	if (allow != 1)
	{

		if (calc_num < 2)
		{
			Last_output_valuel = output_valuel;
		}

		if (Last_output_valuel <= output_valuel)
		{
			patience_T = patience_T + 1;
			if (patience_T > 10)
			{
				step_T = step_T2;
			}
			if (patience_T > patience_max_T)
			{
				out = 1;
			}

		}
		if (Last_output_valuel > output_valuel)
		{
			patience_T = 0;
			Last_output_valuel = output_valuel;
		}

		if (calc_num > max_clac_T)
		{
			out = 1;
		}


	}
	return out;

}
int get_up(float x)
{
	int i = x;
	float y = x - i;
	if (y > 0)
	{
		return i + 1;
	}
	else
	{
		return i;
	}


}
int calc_output_value()  //计算T_target和分数
{
	int T, customer, site;
	getvalueT = time_num;
	int output_value = 0;
	for (site = 0; site < real_site_num; site++)
	{
		T_target[site] = band_work_sort_index[site][getvalueT - 1];  //95的索引是94
		output_value = output_value + band_work[site][getvalueT - 1];
	}
	return output_value;
}

void calc_gravitation_list_for_Time() //计算搬运矩阵
{
	//求解的列和
	int i, j;
	int site;
	clc(gravitation_list, real_site_num);
	for (i = 0; i < out_site_num; i++)
	{
		for (site = 0; site < real_site_num; site++)
		{
			if (site == out_site[i])
			{
				gravitation_list[site] = 1;//1代表输出节点
			}
			if (site == in_site[i])
			{
				gravitation_list[site] = -1;//-1代表输入节点
			}
		}
	}
}


int cala_need_step()//计算需要搬走的量
{
	int site, band, band_last_one, move_band, move_band_max;
	for (site = 0; site < out_site_num; site++)
	{
		//拿出95的量
		band = band_work[site][getvalueT - 1];
		band_last_one = band_work[site][getvalueT - 2];
		move_band = band - band_last_one;
		if (site == 0)
		{
			move_band_max = move_band;
		}
		else
		{
			if (move_band >= move_band_max)
			{
				move_band_max = move_band;
			}
		}

	}
	return move_band_max;


}
int move_band_max;


void calc_transport_list_for_Time(int T, int customer) //计算优化解所用的搬运矩阵
{
	int i, j;
	clc(transport_list, real_site_num);//清零
	clc(max_moving_band, real_site_num);

	for (j = 0; j < real_site_num; j++)
	{
		slove_one_Mat_clo_sum[j] = 0;
		for (i = 0; i < customer_num; i++)
		{
			slove_one_Mat_clo_sum[j] = slove_one_Mat_clo_sum[j] + slove_Mat_all[T][i][j];
		}
	}//要计算最大值

	for (i = 0; i < real_site_num; i++)
	{
		max_moving_band[i] = SB_data[real_site_index[i]] * 0.95 - slove_one_Mat_clo_sum[i];
		if (max_moving_band[i] <= 0)
		{
			max_moving_band[i] = 0;
		}
	}
	for (i = 0; i < real_site_num; i++)
	{
		if (real_allow_data[customer][i] != 0)
		{
			for (j = 0; j < real_site_num; j++)
			{

				if ((real_allow_data[customer][j] != 0) && (i != j))
				{
					gravitational = -1 * (gravitation_list[i] - gravitation_list[j]);
					if (gravitational == -2)
					{
						gravitational = -0.1;
					}
					if (gravitational == 2)
					{
						gravitational = 0.1;
					}
					else
					{
						gravitational = 0;
					}
					//只有2和-2可以用
					transport_band = (gravitational * step_T);
					if (transport_band != 0)
					{
						int test = 1;
					}
					if ((transport_band < 0) && (slove_Mat_all[T][customer][i] + transport_band + transport_list[i] <= 0))
					{
						// 如果ia搬走的太多，就不行，只能搬走还剩下的
						move_band = slove_Mat_all[T][customer][i] + transport_list[i];
						if (move_band < 0)
						{
							move_band = -1 * move_band;
						}
						transport_list[i] = transport_list[i] - move_band;
						transport_list[j] = transport_list[j] + move_band;
					}

					else if ((transport_band > 0) && (slove_Mat_all[T][customer][j] - transport_band + transport_list[j] <= 0))
					{
						//如果ma搬走了太多，也要只能搬走剩下了的
						move_band = slove_Mat_all[T][customer][j] + transport_list[j];
						if (move_band < 0)
						{
							move_band = -1 * move_band;
						}
						transport_list[i] = transport_list[i] + move_band;
						transport_list[j] = transport_list[j] - move_band;
					}
					else if ((transport_band < 0) && (-1 * transport_band + transport_list[j] >= max_moving_band[j]))

					{ // 也不能一进去太多，不然也会超上限
						move_band = max_moving_band[j] - transport_list[j];
						transport_list[i] = transport_list[i] - move_band;
						transport_list[j] = transport_list[j] + move_band;
					}
					else if ((transport_band > 0) && (transport_band + transport_list[i] >= max_moving_band[i]))

					{ // 也不能一进去太多，不然也会超上限
						move_band = max_moving_band[i] - transport_list[i];
						transport_list[i] = transport_list[i] + move_band;
						transport_list[j] = transport_list[j] - move_band;
					}
					else
					{
						transport_list[i] = transport_list[i] + transport_band;
						transport_list[j] = transport_list[j] - transport_band;
					}
				}

			}
		}


	}
}

int best_output_valuel;
void slove_mat_optimization()
{
	//进行矩阵优化
	//计算节点和时间的矩阵
	float ratio;
	int T, customer, site;
	int calc_time = 0;
	int i,z,same;
	int allow;

	//进行迭代
	while (true)
	{

		clc(in_site, 135);
		clc(out_site, 135);
		for (T = 0; T < time_num; T++)
		{
			for (site = 0; site < real_site_num; site++)
			{
				band_work[site][T] = 0;
				for (customer = 0; customer < customer_num; customer++)
				{
					band_work[site][T] = band_work[site][T] + slove_Mat_all[T][customer][site];
				}
			}
		}
		//进行排序
		for (site = 0; site < real_site_num; site++)
		{
			BubbleSort(band_work[site], time_num, band_work_sort_index[site]);
		}
		output_valuel = calc_output_value();

		if (check_slove_out_for_T(choose_time, calc_time) == 1)
		{
			break;
		}

		

		//计算95最多站点
		clc(T_95_sum, time_num);
		for (site = 0; site < real_site_num; site++)
		{
			T_95_sum[T_target[site]] = T_95_sum[T_target[site]] + 1;
		}
		choose_time = max_index(T_95_sum, time_num); //选择迭代的时间

		out_site_index = 0;
		for (site = 0; site < real_site_num; site++)
		{
			if (T_target[site] == choose_time)
			{
				out_site[out_site_index] = site;
				out_site_index = out_site_index + 1;
			}
		}
		out_site_num = out_site_index;
		//得到输出节点
		in_site_index = 0;
		for (site = 0; site < real_site_num; site++)
		{
			if (band_work_sort_index[site][choose_time] > getvalueT-1)
			{
				in_site[in_site_index] = site;
				in_site_index = in_site_index + 1;
			}
		}
		in_site_num = in_site_index;//得到输入节点
		if (in_site_num < out_site_num*3) //如果输入节点少于输出节点，找一个前面的结点
		{
			
			for (ratio = 0.05; ratio<0.70; ratio = ratio+0.05)
			{ 
				if (in_site_num == out_site_num*3)
				{
					break;
				}

				for (site = 0; site < real_site_num; site++)
				{
					int site_connect = 0;
					same = 0;
					for (z = 0 ; z < out_site_num; z++)
					{
						if (site == out_site[z])
						{
							same = 1;
						}
						for (int customer_1 = 0; customer_1 < customer_num; customer_1++)
						{
							if (real_allow_data[customer_1][out_site[z]] == 1)
							{
								site_connect = site_connect + real_allow_data[customer_1][site];
							}
						}
					}
					for (z = 0; z < in_site_num; z++)
					{
						if (site == in_site[z])
						{
							same = 1;
						}
					}
					if ((band_work_sort_index[site][choose_time] < time_num * ratio)&&(same==0)&&(site_connect> 3))//起码联通的
					{
						in_site[in_site_index] = site;
						in_site_index = in_site_index + 1;
						in_site_num = in_site_num + 1;
						if (in_site_num == out_site_num*3)
						{
							break;
						}
					}
				}
			}
		} //找到了点

		for (customer = 0; customer < customer_num; customer++)
		{
			//move_band_max = (cala_need_step() / customer_num) + 5;
			calc_gravitation_list_for_Time(); //计算重力矩阵
			calc_transport_list_for_Time(choose_time, customer); //计算搬运矩阵
			for (site = 0; site < real_site_num; site++)
			{
				slove_Mat_all[choose_time][customer][site] = slove_Mat_all[choose_time][customer][site] + transport_list[site];
			}
		}
		/*cout << "choose_time: " << choose_time << endl;
		cout << "output_valuel: " << output_valuel << endl;
		cout << "patience_T: " << patience_T << endl;
		cout << "calc_time: " << calc_time << endl;
		cout << "out_site: " ;
		for (z = 0; z < out_site_num; z++)
		{
			cout << out_site[z] << ";";

		}
		cout << endl;

		cout << "in_site: ";
		for (z = 0; z < in_site_num; z++)
		{
			cout << in_site[z]<<";";

		}
		cout << endl;*/

		
		calc_time = calc_time + 1;


	}



}

string trimstr(string s) {
	size_t n = s.find_last_not_of(" \r\n\t");
	if (n != string::npos) {
		s.erase(n + 1, s.size() - n);
	}
	n = s.find_first_not_of(" \r\n\t");
	if (n != string::npos) {
		s.erase(0, n);
	}
	return s;
}

void write_output()
{
	int T, customer, site;
	int first_site = 0;
	ofstream outfile(output_path, ios::trunc);
	for (T = 0; T < time_num; T++)
	{
		for (customer = 0; customer < customer_num; customer++)
		{

			outfile << trimstr(customers_name[customer]) << ":";
			first_site = 0;
			for (site = 0; site < real_site_num; site++)
			{
				if (slove_Mat_all[T][customer][site] != 0)
				{
					if (first_site != 0)
					{
						outfile << ",";
					}
					outfile << "<" << trimstr(sites_name[real_site_index[site]]) << "," << slove_Mat_all[T][customer][site] << ">";
					first_site = 1;
				}
			}
			if (((T != time_num - 1) || (customer != customer_num - 1)))
			{
				outfile << "\n";
			}

		}
	}
}

int main()
{
	Read_data();//读取数据
	site_fifter();//筛选节点
	slove_mat(); //计算解
	slove_mat_optimization();
	write_output(); //输出
	exit(0);
}


