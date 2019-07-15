#include "head.h"
int tranFile(int newfd,char* FILENAME)
{
	Train_t train;
	//发送文件名
	train.dataLen=strlen(FILENAME);
	strcpy(train.buf,FILENAME);
    int ret=sendCycle(newfd,(char*)&train,4+train.dataLen);//4是文件名的长度
	ERROR_CHECK(ret,-1,"sendname");
    int fd=open(FILENAME,O_RDONLY);//打开服务器上该文件名对应的文件
	ERROR_CHECK(fd,-1,"open");
    //发送文件大小；
    struct stat buf;
    fstat(fd,&buf);
    train.dataLen=sizeof(buf.st_size);//获取文件大小的大小并赋值给火车头
    memcpy(train.buf,&buf.st_size,train.dataLen);//赋值文件大小
    ret=sendCycle(newfd,(char*)&train,4+train.dataLen);
    ERROR_CHECK(ret,-1,"sendsize");
    //map映射文件
    char* pmap=(char*)mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0);
    ERROR_CHECK(pmap,(char*)-1,"mmap");
    //发文件内容
    ret=sendCycle(newfd,pmap,buf.st_size);
    ERROR_CHECK(ret,-1,"sendCycle");
	//发送结束标志
    train.dataLen=0;
    ret=sendCycle(newfd,(char*)&train,4);
    ERROR_CHECK(ret,-1,"sendover");
	return 0;
}

int getFile(int sfd,off_t seek)
{
    int RECV_BLOCK=65536;
	int dataLen,recvCount;
    char buf[1000]={0};
    //接收文件名
	recvCycle(sfd,(char*)&dataLen,4);//得到文件名大小
	recvCycle(sfd,buf,dataLen);//接收文件名
    printf("Filename is <%s>\n",buf);	
    int fd=open(buf,O_CREAT|O_RDWR,0666);
	ERROR_CHECK(fd,-1,"open");
    lseek(fd,seek,SEEK_SET);
    //接收文件大小
    off_t fileSize=0,downloadSize=0,oldSize=0;
    recvCycle(sfd,(char*)&dataLen,4);
    recvCycle(sfd,(char*)&fileSize,dataLen);
    printf("The size of <%s> = %ld\n",buf,fileSize);
    fileSize-=seek;
    off_t sliceSize=fileSize/RECV_BLOCK;
    
    //接收文件内容
    int fds[2];
    pipe(fds);//splice无法从socket直接向fd传文件，需通过管道
    if(fileSize>RECV_BLOCK){
        while(downloadSize+RECV_BLOCK<fileSize){
            recvCount=splice(sfd,NULL,fds[1],NULL,RECV_BLOCK,SPLICE_F_MORE|SPLICE_F_MOVE);
            ERROR_CHECK(recvCount,-1,"splice1");
            recvCount=splice(fds[0],NULL,fd,NULL,recvCount,SPLICE_F_MORE|SPLICE_F_MOVE);
            ERROR_CHECK(recvCount,-1,"splice2");
            downloadSize+=recvCount;
            if(downloadSize-oldSize>sliceSize){//如果下载增量大于了最小进度单位
                printf("\r%5.2f%% ",(float)downloadSize/fileSize*100);
                for(off_t i=0;i<(downloadSize*50)/fileSize;i++){
                    printf("*");
                }
                fflush(stdout);//清除缓冲区}
                oldSize=downloadSize;
            }
        }
    }
    recvCount=splice(sfd,NULL,fds[1],NULL,fileSize-downloadSize,SPLICE_F_MORE|SPLICE_F_MOVE);
    ERROR_CHECK(recvCount,-1,"splice1");
    recvCount=splice(fds[0],NULL,fd,NULL,recvCount,SPLICE_F_MORE|SPLICE_F_MOVE);
    ERROR_CHECK(recvCount,-1,"splice2");
    printf("\r100.00%%\nDownload compelete!\n");
    
    //接结束符
    recvCycle(sfd,(char*)&dataLen,sizeof(int));
    close(fd);
    return 0;
}

