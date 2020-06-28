#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

sem_t musteri,berber,mutex;
int musteriSayisi;
int *musteriBilgileri;

typedef struct BagliListeDugumu //Liste Dugumu
{
    int bekleyenMusteriId; // Verinin kendisi
    struct BagliListeDugumu* next; // Listedeki sonraki eleman
} Node;

typedef struct BagliListe //Bekleme salonuna alinan kisileri, ayni sirayla tras etmek icin bagli liste yapisi tanimlandi.
{   
    Node* head;
} Liste;

Liste* bekleyenMusteriListesi;

void elemanEkle(Liste* list, Node* eleman) //Bekleme listesine eleman ekleme
{
Node* gezinmeNode=list->head;
if(gezinmeNode==NULL) //Ilk eleman yoksa ilk eleman yap
	{
	list->head=eleman;
	Node* eklenen=list->head;
	int sayi=eklenen->bekleyenMusteriId;
	}
else
	{
	while(gezinmeNode->next!=NULL) //Listenin sonuna kadar ilerle
		gezinmeNode=gezinmeNode->next;
	gezinmeNode->next=eleman; //Listenin sonuna ekle
	Node* eklenen=gezinmeNode->next;
	int sayi=eklenen->bekleyenMusteriId;
	}
}

int elemanCikar(Liste* list) //Listeden eleman cikarma
{
int sayi=0;
Node* gezinmeNode=list->head;
Node* yenihead;
sayi=gezinmeNode->bekleyenMusteriId; //Listenin basindaki elemani cikar ve bir sonraki elemani listenin basi yap.
yenihead=gezinmeNode->next;
list->head=yenihead;
gezinmeNode=NULL;
return sayi;
}

int elemanSayisiBulma(Liste* list) //Listedeki eleman sayisini bulma
{
int sayi=0;
if(list==NULL) //Bekleyen kimse yoksa 0 dondurulur.
	return sayi;
Node* gezinmeNode=list->head;
if(gezinmeNode==NULL) //Bekleyen kimse yoksa 0 dondurulur. 
	return sayi;
else
{
	while(gezinmeNode->next!=NULL) //Listenin sonuna kadar gidilir ve kac eleman varsa geri dondurulur.
	{gezinmeNode=gezinmeNode->next;
	sayi++;}
	return sayi+1;
}
}

void initialize() //Semaforlar tanimlanir.
{
sem_init(&musteri,0,1);
sem_init(&berber,0,1);
sem_init(&mutex,0,1);
bekleyenMusteriListesi=malloc(sizeof(Liste)); //Bekleme sirasindakiler icin liste burada tanimlaniyor.
}

void musteriBilgisiAl() //Kac musteri olacagi kullanicidan alinir.
{
printf("Musteri sayisini giriniz: \n");
scanf("%d",&musteriSayisi);
musteriBilgileri=(int*)malloc(sizeof(int)*musteriSayisi);  //Berber dukkanina girecek musteriler icin bir dizi olusturulur ve hepsine bir Id tanimlanir.
for(int i=0;i<musteriSayisi;i++)
	musteriBilgileri[i]=rand()%1000;
}

void berberProses() //Berber Prosesi.
{
printf("\nBerber uyandi\n");
sleep(2);
do
{
	if(elemanSayisiBulma(bekleyenMusteriListesi)!=0) //Bekleyen musteri varsa musteri tras islemi gerceklestirilir.
	{
	sem_wait(&berber); //Berber musterisini kabul eder.
	sem_wait(&mutex); //Musterinin,musteri ve berber prosesinin ortak olarak kullanilan veri olan bekleyen musterilere ayni anda erismemesi saglanir.
	int trasOlanEleman=elemanCikar(bekleyenMusteriListesi);
	sem_post(&mutex); //Erisim engeli kaldirilir.
	sem_post(&musteri); //Bos olan berber koltugu oldugu bildirilir.
	printf("\nBerber %d numarali musteriyi tirasa basladi.\n",trasOlanEleman);
	sleep(3);  //Musteri burada tras oluyor.
	printf("\n%d numarali musteri tiras oldu ve dukkandan ayriliyor.\n",trasOlanEleman);
	sleep(1);
	}
	else //Salonda bekleyen musteri yoksa berber uyur.
	{
	printf("\nTras edecek kimse yok. Berber uyuyor\n");
	sleep(5);
	}
}while(1);
pthread_exit(0);
}

void musteriProses(void*  veri) //Musteri Prosesi.
{
int musteriId=*((int*)veri);
int bekleyenKisi=elemanSayisiBulma(bekleyenMusteriListesi); //Sirada kac kisi bekledigi bulunur.
sem_wait(&mutex); //Berberin, musteri ve berber prosesinin ortak olarak kullanilan veri olan bekleyen musterilere ayni anda erismemesi saglanir.
printf("\nSalona %d numarali musteri girdi\n",musteriId);
if(bekleyenKisi!=5) //Salondaki bos sandalye varsa, musteri oturur.
	{
	Node* node=malloc(sizeof(Node));
	node->bekleyenMusteriId=musteriId;
	elemanEkle(bekleyenMusteriListesi,node); //Bekleme salonuna bir musteri ekleniyor.
	sleep(1);
	printf("\nBos sandalyeye %d numarali musteri oturdu.\n",musteriId);
	printf("\n%d kisi sirada bekliyor\n",bekleyenKisi+1);
	sem_post(&mutex); //Erisim engeli kaldirilir.
	sem_post(&berber); //Berber uyuyorsa uyandirilir.
	sem_wait(&musteri); //Musteri bostaki berberi bulur ve berberin tek bir musteriyi tras etmesini saglar.
	}
else //Salonda bos sandalye kalmamissa musteri salondan ayrilir.
	{
	printf("\nOturacak bos sandalye yok. %d numarali musteri cikis yapiyor\n",musteriId);
	sem_post(&mutex); //Erisim engeli kaldirilir.
	}
	sleep(1);
pthread_exit(0);
}

void main()
{
musteriBilgisiAl(); //Kullanicidan musteri sayisi alinir.
initialize(); //Threadler ve bekleme listesi olusturulur.
pthread_t berberler[1], musteriler[musteriSayisi]; //Threadler tanimlandi.

for(int i=0;i<1;i++)
{
pthread_create(&berberler[1],NULL,(void*)berberProses,NULL); //Berber threadi olusturuldu.
sleep(1);
}
for(int i=0;i<musteriSayisi;i++)
{
pthread_create(&musteriler[musteriSayisi],NULL,(void*)musteriProses,(void*)&musteriBilgileri[i]); //Musteri threadleri olusturuldu.
sleep(1);
}
for(int i=0;i<musteriSayisi;i++)
{
pthread_join(musteriler[musteriSayisi],NULL); //Butun musteri threadleri teker teker calistirilir.
sleep(15);
}
}
