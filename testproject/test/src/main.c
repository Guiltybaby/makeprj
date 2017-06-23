#include <stdio.h>
#include <pthread.h>
extern void*malloc(size_t size);


//sn = a1(1-q^n)/(1-q)

float power(float input,int power)
{
	int i = 0;
	float total = 1;
	for(i = 0; i < power;i ++ )
	{
		total*= input;
	}
	return total;
}

float dengbi(float radio,int amount,int times)
{
	float total = amount * (1 - power(radio,times))/(1 - radio);
	return total;
}

#define ABS(a,b) ((a)>(b)?((a)-(b)):((b)-(a)))

int calulate_timer_earning_radio(float total,int amount, int times )
{
	float tryradio = total/amount/times;
	float radio = 0;
	float try_total = 0;	
	float det = 0.0000001;
	while(ABS(total,try_total)> 0.1)
	{
		try_total = dengbi(tryradio,amount,times);
		if(try_total > total)
		{
			tryradio -= det;
		}else{
			tryradio += det;
		}
		printf("tryradio = %4.4f%% total = %f try_total = %6.2f \n",(tryradio*100 - 100),total,try_total);
		continue;	
	}	
	printf("anual = %4.2f%% \n",(power(tryradio,53) - 1)*100 );
}

void* worker_thread()
{
	printf("aaa \n");
}
void main()
{
	void* a;
	printf("111`````` \n");
	a = malloc(111);
	printf("222``````\n");
	float total = 7074.71;
	int amount = 1000;
	int times = 7;
//	calulate_timer_earning_radio(total,amount,times);

	size_t stacksize = 0;
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
//	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_getstacksize(&attr,&stacksize);
	int rc = pthread_create(&tid,&attr,worker_thread,NULL);

	while(1){
		sleep(1);
		printf("xxx \n");
	}

}
