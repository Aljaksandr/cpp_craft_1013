#include <iostream> 
#include <fstream>
#include <string> 
#include <algorithm>
#include <list>
#include <iomanip>
#include <sstream>
#include <set>

using namespace std;

typedef set<int> list_of_numbers;

int main() 
{ 
	int N = 0;
	int number;
	string string_number;
	list_of_numbers keys;
	fstream input_file, output_file;
	stringstream ss;


	input_file.open("Input.txt", fstream::in);
	if(input_file)
	{
		output_file.open("Output.txt", fstream::out);
		input_file >> N;
	
		int i = 0;
		// ������ ����� � ������
		input_file >> string_number;
		for(;!input_file.eof(); i++)
		{
			// ���� �����, ���� ������� �� ������� ������� �������� ����� �����
			// ���� �� ������ 4 �� ������� �� 4 ���������
			// � ��������� ������ ��������� ���� �� ���������� 4 �������� ����� �����
			size_t found = string_number.find(".");
		
			if(found != string::npos)
			{
				if(string_number.size() - found - 1 <= 4)
				{
					string_number.append(4 - (string_number.size() - found - 1), '0');
				}
				else 
				{
					string_number.erase( found + 5, string_number.size() - 1);
				}
				// ������� �����
				string_number.erase(string_number.begin() + found);
			}
			else // ����� ����� ������� ��������� 4 ���� � �����
			{
				string_number.append(4 , '0');
			}
			// ��������� ������ � ����� �����
			istringstream ( string_number ) >> number;
			// ����� ��������� �� ���������
			if( i < N)
			{
				keys.insert(number);
			}
			else // ���� ����� ����������� �� ���� ����������
			{
				if(keys.find(number) != keys.end())
				{
					output_file << "YES" <<endl;
				}
				else
				{
					output_file << "NO" <<endl;
				}
			}
			input_file >> string_number;
		}

		input_file.close();
		output_file.close();
	}
}