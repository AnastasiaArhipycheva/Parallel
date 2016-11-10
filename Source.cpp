#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <iostream>
using namespace std;

void Transpose_matrix(int rows, int cols, int *A_local, int *AT_local)
{

	for (int i = 0; i < cols; i++)
		for (int j = 0; j < rows; j++)
			AT_local[j + i*rows] = A_local[i + cols*j];
}

void Fill_matrix(int rows, int cols, int *A, int *AT) {


	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			A[j + cols*i] = rand() % 10;
				//cout << A[j + cols*i];

		}
		//	cout << endl;
	}

	for (int i = 0; i < cols; i++)
		for (int j = 0; j < rows; j++)
			AT[j + i*rows] = 0;

}

int main(int argc, char *argv[])
{
	if (argc < 3)
		return -1;

	int procRank, procNum;
	int *size, *displs, offset;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	int rows, cols;
	int *A = 0, *AT = 0;
	int *A_local = 0, *AT_local = 0;
	double starttime, endtime;

	srand(time(NULL));

	if (procRank == 0)
	{
		cout << "Rows" << endl;
		cin >> rows;

		cout << "Cols" << endl;
		cin >> cols;

		A = new int[rows*cols];

		AT = new int[rows*cols];


		Fill_matrix(rows, cols, A, AT);
		starttime = MPI_Wtime();
	}




	MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//Определяем количество столбцов, которые будут отправлены на каждый процесс
	int c = cols / procNum;

	offset = 0;
	size = new int[procNum];
	for (int i = 0; i < procNum; i++)
		size[i] = c;
	int mod = cols%procNum;
	int j = 0;
	while (mod > 0)
	{
		size[j]++;
		mod--;
		j++;
		if (j >= procNum)
			j = 0;
	}

	displs = new int[procNum];
	for (int i = 0; i < procNum; i++)
	{
		displs[i] = offset;
		offset += size[i];
	}
	A_local = new int[rows*size[procRank]];
	AT_local = new int[rows*size[procRank]];

	//Рассылка столбцов
	for (int i = 0; i < rows; i++)
		MPI_Scatterv(&(A[i*cols]), size, displs, MPI_INT,
			&(A_local[i*size[procRank]]), size[procRank], MPI_INT, 0, MPI_COMM_WORLD);

	




	//Транспонирование на каждом процессе
	Transpose_matrix(rows, size[procRank], A_local, AT_local);

	offset = 0;
	for (int i = 0; i < procNum; i++)
		size[i] *= rows;
	for (int i = 0; i < procNum; i++)
	{
		displs[i] = offset;
		offset += size[i];
	}

	//Собираем результат
	MPI_Gatherv(AT_local, size[procRank], MPI_INT, AT, size, displs, MPI_INT, 0, MPI_COMM_WORLD);

	//Вывод результата
	if (procRank == 0)
	{

		endtime = MPI_Wtime();


		double time = endtime - starttime;
		printf("Parallel runtime: %f \n", time);

		//for (int i = 0; i < rows; i++) {
		//	for (int j = 0; j < cols; j++) {
		//		cout << AT[j + cols*i];
		//	}
		//	cout << endl;
		//}


	}

	MPI_Finalize();

	delete[]A;
	delete[]AT;
	delete[]A_local;
	delete[]AT_local;
	delete[]size;
	delete[]displs;


	return 0;
}