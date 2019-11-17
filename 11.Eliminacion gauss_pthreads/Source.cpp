#define _TIMESPEC_DEFINED 
/*https://stackoverflow.com/questions/33114535/timespec-struct-type-error-c2011?noredirect=1&lq=1*/
#include <pthread.h> 
#include <cstdio>
#include <iostream>
#include<chrono>
#include<cstdlib>
#include<time.h>
#include<stdbool.h>


using namespace std;
int MAX_NUM = 40;
int DIM_MAT = 400;

struct limPar
{
	double **mat;  // puntero de matriz
	int inicio;  // columna de celda inicial
	int fin;     // columna de celda final
	int nPivote; // renglon del pivote
	int nRow;    // renglon al eliminar
	double pivote;  // valor pivote
	
};

typedef struct limPar limPar;

double aleat() // aleatorio entre 1 y MAX_NUM
{
	
	return  (double) (rand()%MAX_NUM +1)/1;
}

double **MakeMatrix(int dim) // Crear matriz dinamica aleatoria (dimxdim) 
{
	double **mat = new double *[dim];
	for (int i=0;i<dim;i++)
	{
		mat[i] = new double[dim];
		for (int j=0;j<dim;j++)
		{
			mat[i][j] =  aleat();
		}
	
	}
	return mat;
}

double **CopyMatrix(int dim, double **mat)
{
	double **copyMatrix = new double *[dim];
	for (int i = 0; i < dim; i++)
	{
		copyMatrix[i] = new double[dim];
		for (int j = 0; j < dim; j++)
		{
			copyMatrix[i][j] = mat[i][j];
		}

	}
	return copyMatrix;

}

void swap_row(double **mat, int dim, int col, int pivote) // permutar columnas
{
	int i;
	double aux;
	for (i=col;i<dim;i++)
	{
		aux = mat[col][i];
		mat[col][i] = mat[pivote][i];
		mat[pivote][i] = aux;
	
	}
}


void pivoteo(double **mat, int col,int dim) // Pivotear matriz
   {
	double aux;
	if(col<dim-1)
	{
		int pivote = col;
		for (int i = col; i < dim; i++)
		{
			if (mat[i][col] > mat[col][col])
			{
				pivote = i;
			}
		}// fin for
		if (pivote != col)
		{
			swap_row(mat, dim, col, pivote); // permutar renglón
		}	
			
	} // fin if
	
		aux = mat[col][col];
	for (int i = col; i < dim; i++)
	{
		 
		mat[col][i] = (double)mat[col][i]/ aux; // dividir renglon entre pivote
		

	}
   
}

void eliminar_renglon_seq(double **mat, int ncol, int dim) // Eliminacion secuencial
{
	for(int i=ncol+1;i<dim;i++)
	{
		double valPivot = mat[i][ncol];
		for(int j=ncol;j<dim;j++)
		{
			mat[i][j] = mat[i][j] - mat[ncol][j] * valPivot;
		}
	}
}

void *eliminar_paralelo(void *args) // Eliminacion paralela ejecucion de hilos
{
	limPar *data = (limPar *) args;
	int row = data->nRow;
	int pivote = data->nPivote;
	for (int i = data->inicio; i < data->fin+1; i++)
	{
		data->mat[row][i] = data->mat[row][i] - data->mat[pivote][i] * data->pivote;
		 
	}
	pthread_exit(0);
	return NULL;
}

void eliminar_renglon_par(double **mat, int ncol, int dim, int nThreads=2) 
{
	// Eliminacion paralela creacion hilos
	
	if (ncol < dim - nThreads)  // verificar si es viable paralelizar
	{
		
		pthread_t *threads = new pthread_t[nThreads];
		int incremento = (dim - ncol - 1) / nThreads - 1;
		limPar  *data = new limPar[nThreads];
		
		for (int i = ncol + 1; i < dim; i++)
		{
			int arranque = ncol;
			double valPivot = mat[i][ncol];
			
			
			for (int j = 0; j < nThreads; j++)
		    {
				data[j].pivote = valPivot;
				data[j].nPivote = ncol;
				data[j].nRow = i;
				data[j].inicio = arranque;
				arranque += incremento;
				if (j < nThreads - 1)
				{
					data[j].fin = arranque;
				}
				else
				{
					data[j].fin = dim - 1;
				}
				data[j].mat = mat;
				arranque++;
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_create(&threads[j], &attr, eliminar_paralelo, &data[j]);
			}//fin for j (generar hilos)

			for (int j = 0; j < nThreads; j++)
			{
				pthread_join(threads[j], NULL);

			} // terminar hilos

			
			
		}// for i (renglones)
		delete data;
		delete threads;
		
	}else // si no es viable paralelizar se continua de forma secuencial
	{
		eliminar_renglon_seq(mat, ncol, dim);
	}

}

void eliminacion_gauss( double **mat,int dim, bool paralelo)
{
	int ncol;
	for (ncol=0;ncol<dim;ncol++)
	 {
		pivoteo(mat,ncol,dim);
		
		
		
		if(ncol<dim-1)
		{
		
			if (paralelo)
			{
				eliminar_renglon_par(mat, ncol, dim,2);
			}
			else
			{
				eliminar_renglon_seq(mat, ncol, dim);
			}
		}
		
		
	}

}



int main()

{
	int dim = DIM_MAT;
	double **mat_test = MakeMatrix(dim);
	double **mat_test2 = CopyMatrix(dim, mat_test);

	cout.precision(2);
	cout << endl << endl << "REDUCCION GAUSS" << endl;
	cout << "Matriz generada con dimension: " << dim << endl << endl;
	for(int i=0;i<dim;i++)
	{
		for (int j = 0; j < dim; j++)
		{
			//cout << mat_test[i][j] << " ";
		}
		//cout << endl;
	}

	auto t1 = std::chrono::high_resolution_clock::now();
	eliminacion_gauss(mat_test, dim, false);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	
	

	cout << endl << endl<<"Reduccion Gauss (SECUENCIAL)"  << endl << endl;
	printf("Tiempo de ejecucion \t %d microsec \n\n", duration);
	for (int i = 0; i < dim; i++)
	{
		for (int j = 0; j < dim; j++)
		{
			//cout << mat_test[i][j] << " ";
		}
		//cout << endl;
	}

	
	auto t3 = std::chrono::high_resolution_clock::now();
	eliminacion_gauss(mat_test2, dim, true);
	auto t4 = std::chrono::high_resolution_clock::now();
	auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
	
	

	cout << endl << endl << "Reduccion Gauss (PARALELO)" << endl << endl;
	printf("Tiempo de ejecucion \t %d microsec \n\n", duration2);
	for (int i = 0; i < dim; i++)
	{
		for (int j = 0; j < dim; j++)
		{
			//cout << mat_test2[i][j] << " ";
		}
		//cout << endl;
	}

	for (int i = 0; i < dim; i++)
	{
		delete[]  mat_test[i];
		delete[]  mat_test2[i];
	}
	delete mat_test;
	delete mat_test2;

	system("pause");
	return 0;
}