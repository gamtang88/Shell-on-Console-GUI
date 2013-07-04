#include <stdio.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <sys/stat.h> // _stati64()
#include <io.h>




typedef struct FileList{
	char name[20];
	char full_name[_MAX_PATH];
	char maked_data[50];
	__int64 size;
	int checked;
	int is_file;
}FileList;

typedef struct Selected{
	char name[17];
	char full_name[_MAX_PATH];
	struct Selected* link;
}Selected;

__int64 getFileSize(char *filename) {
	struct _stati64 statbuf;

	if ( _stati64(filename, &statbuf) ) return -1; // 파일 정보 얻기: 에러 있으면 -1 을 반환

	return statbuf.st_size;                        // 파일 크기 반환
}

int isFileOrDir(char* s) {
	struct _finddatai64_t c_file;
	intptr_t hFile;
	int result;


	if ( (hFile = _findfirsti64(s, &c_file)) == -1L )
		result = -1; // 파일 또는 디렉토리가 없으면 -1 반환
	else if (c_file.attrib & _A_SUBDIR)
		result = 0; // 디렉토리면 0 반환
	else
		result = 1; // 그밖의 경우는 "존재하는 파일"이기에 1 반환


	_findclose(hFile);

	return result;
}



int get_list(FileList f_list[]){
	struct _finddata_t fd;
	intptr_t handle;
	int f_count=0,i=0;

	if((handle = _findfirst("*.*",&fd))==-1L) return -1;
	else{
		do{
			while(fd.name){
				if(i==20)break;
				if(i<17) f_list[f_count].name[i]=fd.name[i];
				else{
					f_list[f_count].name[17]='.';f_list[f_count].name[18]='.';f_list[f_count].name[19]='\0';//뒤는 ..으로 생략
				}
				i++;
			}i=0;
			strcpy(f_list[f_count].full_name,fd.name);
			f_list[f_count].size=getFileSize(fd.name);
			f_list[f_count].is_file=isFileOrDir(fd.name);
			//f_list[f_count].size=fd.size;
			ctime_s(f_list[f_count].maked_data,sizeof(f_list[f_count].maked_data),&(fd.time_write));
			while(f_list[f_count].maked_data[i+1])i++;//날짜는 맨끝이 개행문자라서 널로 바꿈
			f_list[f_count].maked_data[i]='\0';i=0;
			if(f_list[f_count].checked !=1) f_list[f_count].checked=0; //체크된게 아니면 0으로 초기화
			f_count++;
		}while(_findnext(handle,&fd)==0);
		_findclose(handle);
	}
	return f_count;
}

void print_f(FileList current_f,int in_detail,Selected* temp){
	int i;
	int dis_size;
	printf("%s",current_f.checked==0?"○":"●");
	printf("%20s",current_f.name);
	printf("%6s",current_f.is_file?" [D]":" [F]");
	if(current_f.size>1000000){//MB일때
		dis_size=current_f.size/1000000;
		printf("┃%9dM┃",dis_size);
	}
	else if(current_f.size>1000){//KB일때
		dis_size=current_f.size/1000;
		printf("┃%9dK┃",dis_size);
	}
	else
		printf("┃%10d┃",current_f.size);
	printf("%26s┃",in_detail==1?current_f.maked_data:"");
	printf("%18s┃\n",temp?temp->name:"");
}

void selected_link(Selected** root,FileList f_node){
	Selected* new_node=(Selected*)malloc(sizeof(Selected));//alloction
	Selected* temp=*root;
	int i;
	if(!new_node){
		fprintf(stderr,"할당오류");return;
	}
	if(check_selected(*root,f_node.full_name)){ //중복파일인게 확인되면 리턴
		free(new_node);return; //할당해제하고 리턴
	}
	for(i=0;f_node.name[i];i++){  //출력할 파일 이름 복사
		if(i==16){
			new_node->name[i]='\0';break;
		}
		else if(i>13) new_node->name[i]='.';
		else {
			new_node->name[i]=f_node.name[i]; 
			if(!f_node.name[i+1])new_node->name[i+1]='\0';//다음에 끝나면 널 삽입
		}
	}
	strcpy(new_node->full_name,f_node.full_name);//풀네임 복사
	new_node->link=NULL;
	if(!*root)
		(*root)=new_node;
	else{
		while(temp->link)temp=temp->link;
		temp->link=new_node;
	}
}

int check_selected(Selected* temp,char* full_name){  //중복확인
	while(temp){
		if(!strcmp(temp->full_name,full_name))return 1;
		temp=temp->link; 
	}
	return 0;
}

int delete_selected(Selected** root,int number_of_file){
	Selected* temp=*root,*before=*root;
	int i;
	for(i=0;i<number_of_file;i++){
		temp=*root;
		while(temp->link){
			if(!(temp->link->link))before=temp;
			temp=temp->link;//마지막 노드까지감.
		}
		rmdir(temp->full_name);
		free(temp);//동적메모리 해제
		before->link=NULL;
	}
	*root=NULL;
}


int main(){
	int i=0,j=0,count=0,num=0,down_scroll=0,up_scroll=0,list_end=0,yy=0,xx=0;
	int cusor=0,key,in_detail=0,num_selected_file=0;
	char dir_name[22]="\0";
	FileList f_list[200];
	Selected* root=NULL,*selected_temp=NULL;
	char current_path[_MAX_PATH]="\0";
	while(1){
		count=get_list(f_list);
		system("cls");
		_getcwd(current_path, _MAX_PATH);
		puts("┏━━━━━━━━━━━┓");
		puts("┃      MONG SHELL      ┃");
		fputs("┗━━━━━━━━━━━┛",stdout);
		printf("\n");//여기에 시간출력
		puts("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
		printf("┃%.10s%.90s   ..┃\n","path : ",current_path);
		puts("┣━━┳━━━━━━━━━━━━┳━━━━━┳━━━━━━━━━━━━━┳━━━━━━━━━┫");
		puts("┃선택┃    파일이름            ┃ 크기(kb) ┃        생성날짜          ┃     선택목록     ┃");
		printf("┣%s┻━━━━━━━━━━━━╋━━━━━╋━━━━━━━━━━━━━╋━━━━━━━━━┫\n",(down_scroll)||(up_scroll)?"▲▲":"━━");
		selected_temp=root; //선택된리스트 템프는 루트로 최신화
		for(i=0;i<20;i++){		
			printf("┃%s",i==cusor?"▶":"　");
			if(count>20){//20보다 목록많을때
				if(down_scroll)
					print_f(f_list[i+yy],in_detail,selected_temp);
				else if(up_scroll)
					print_f(f_list[i+xx],in_detail,selected_temp);
				else
					print_f(f_list[i],in_detail,selected_temp);
			}
			else{ //20개 이하일때
				if(i<count)print_f(f_list[i],in_detail,selected_temp);
				else {
					printf("%2s%26s┃%10s┃%26s","","","","");
					printf("┃%18s┃\n",selected_temp?selected_temp->name:"");
				}
			}
			if(selected_temp)//null이 아닐때 만 꼬리잡기
				selected_temp=selected_temp->link;
		}
		if(!list_end)
			printf("┣%s━━━━━━━━━━━━━┻━━━━━┻━━━━━━━━━━━━━┻━━━━━━━━━┫\n",count>19?"▼▼":"━━");
		else
			puts("┣━━━━━━━━━━━━━━━┻━━━━━┻━━━━━━━━━━━━━┻━━━━━━━━━┫");

		puts("┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫");
		printf("┃도움말%84s┃\n","");
		puts("┣─────────────────────────────────────────────┫");
		puts("┃ENTER:들어가기\t\tS:목록선택\t\tESC:선택 해제\t\t            ┃");
		puts("┃C:선택한목록 복사\t\tD:선택한목록 삭제\tN:폴더생성\t\t            ┃");
		puts("┃F1:가로보기\t\tF2:세로보기\t\tF3:상세히보기\t\tF4:간단히보기       ┃");
		puts("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
		printf("count: %d cusor: %d  num: %d 파일:%s\ncurrent path:%s\n",count,cusor,num,f_list[num].name,current_path);
		printf("up_scroll:%d down_scroll:%d end_list:%d",up_scroll,down_scroll,list_end);
		key=_getch();
		switch(key){
		case 80://key down
			cusor++;num++;
			if(count<21){//20개안쪽일땐 모듈라로 커서 돌림
				cusor%=count;
				num=cusor;
			}
			else{
				if(num==count-1)list_end=1;
				if(num==count)num--;
				if(cusor==20){
					cusor=19;//그자리에 머뭄
					up_scroll=0;
					down_scroll=1;
					yy=num-19;
				}
			}
			break;
		case 72://key up
			cusor--;num--;
			if(count<20){//20개 안쪽일땐 커서돌림
				if(cusor==-1)cusor=count-1;
				num=cusor;
			}
			else{//그이상일땐
				if(cusor==-1){
					cusor=0;xx=num;//그자리에 머뭄
					up_scroll=1;down_scroll=0;list_end=0;
					if(num==-1){
						num=0;up_scroll=0;
					}
				}
			}
			break;
		case 13: //enter
			if(num==1){//상위
				_chdir("..");
				_getcwd(current_path, _MAX_PATH);

			}
			else{
				while(current_path[j])j++;
				current_path[j]='\\';current_path[j+1]='\0';j=0;
				if(_chdir(strcat(current_path,f_list[num].full_name)))
					_getcwd(current_path, _MAX_PATH);
			}
			cusor=0;num=0;
			break;
		case 's': //선택
			if(num!=0&&num!=1){
				if(num_selected_file<20){ //20개 이하일때가 추가가능
					selected_link(&root,f_list[num]); //이어줌
					f_list[num].checked=1;
					num_selected_file++;
				}
			}
			break;
		case 'd':
			if(num_selected_file>0){//선택된게 있을때만
				printf("┃%84s┃\n","선택한 목록들을 삭제하시겠습니까?(Y/y입력)");
				key=_getch();
				if(key=='Y'||key=='y') delete_selected(&root,num_selected_file);
			}
			break;
		case 'n':
			puts("");
			puts("폴더 이름을 입력해주세여(20자 이내밖에 안되요)");
			puts("/\\?|""""<>*: 포함일 경우 폴더생성은 무시됩니다");
			fgets(dir_name,22,stdin);
			for(i=0;dir_name[i];i++);dir_name[i-1]='\0';
			mkdir(dir_name);
			fflush(stdin);
			mkdir("gamgamgam");
			break;
		case 61:
			in_detail=1;
			break;
		case 62:
			in_detail=0;
			break;
		}
	}
}

