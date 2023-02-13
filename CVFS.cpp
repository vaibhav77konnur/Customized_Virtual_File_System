

//#################################################################
//
//
//	Customized Virtual File System Application
//
//
//#################################################################


//####################
//
// Header Files
//
//####################

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>

//####################
//
// Defining The Macros
//
//####################

#define MAXINODE 50
#define READ 1
#define WRITE 2

 #define MAXFILESIZE 2048

#define REGULAR 1
#define SPECIAL 2

#define START  0
#define CURRENT 1
#define END 2

//##################################
//
// Creating SuperBlock Structure
//
//##################################

typedef struct superblock
{
    int TotalInodes;        /// initially size is 50 for both
    int FreeInode;
}SUPERBLOCK, *PSUPERBLOCK;


//##################################
//
// Creating Inode Structure
//
//##################################

typedef struct inode
{
    char FileName[50];   // file name
    int InodeNumber;     //inode number
    int FileSize;        // 1024 bytes or 1kb
    int FileActualSize;  // memory allocated when we write the data into it
    int FileType;        // type of file for now it is only  regular type of file or text file
    char*Buffer;         // on windows it stores block number but in this case it stores 1024 bytes
    int LinkCount;       // linking count
    int Referencecount;  //reference count
    int Permission;      // 1 or 2 or 3 read and write permission
    struct inode *next;  // self referential structure used for linkedlist

}INODE, *PINODE, **PPINODE;

//##################################
//
// Creating FileTable Structure
//
//##################################

typedef struct filetable
{
    int readoffset;      // from where to read
    int writeoffset;     // from where to write
    int count;           // remains one throughout the code
    int mode;            // 1, 2, 3
    PINODE ptrinode;     // pointer, Linkedlist point to inode
}FILETABLE, *PFILETABLE;

//##################################
//
// Creating UFDT Structure
//
//##################################

typedef struct ufdt
{
    PFILETABLE ptrfiletable; // create ufdt structure // pointer which points to filetable

}UFDT;

UFDT UFDTArr[50];         // create Array of structure i.e array of pointers
SUPERBLOCK SUPERBLOCKobj; // global variable
PINODE head = NULL;       // global pointer


//######################################################################################
//
//	Function Name	: 	man
//	Input			: 	char *
//	Output			: 	None
//	Description 	: 	It Display The Description For Each Commands
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

void man(char *name)
{
    if(name == NULL)
    {
        return;
    }

    if(strcmp(name, "create") == 0)
    {
        printf("Description : Used to Create New Regular File \n");
        printf("Usage : create File_name Permission \n");
    }
    else if(strcmp(name, "read") == 0)
    {
        printf("Description : Used to read Data from Regular File \n");
        printf("Usage : read File_name No_of_Bytes_to_read\n");
    }
    else if(strcmp(name,"write") == 0)
    {
        printf("Description : Uses to write int regular file \n");
        printf("Usage : write File_name\n After this Enter the Data that we want to Write \n ");

    }
    else if(strcmp(name,"ls") == 0)
    {
        printf("Description : Used to List all information of files");
        printf("Usage : ls\n");
    }
    else if(strcmp(name,"stat") == 0)
    {
        printf("Description : Used To Display Information of File \n");
        printf("Usage : stat File_name\n");
    }
    else if(strcmp(name,"fstat") == 0)
    {
        printf("Description : Used to Display information of File \n");
        printf("stat File_Descriptor\n");
    }
    else if(strcmp(name,"truncate") == 0)
    {
        printf("Description : Used to Remove data from file \n");
        printf("Usage : truncate File_name\n");
    }
    else if(strcmp(name,"open") == 0)
    {
        printf("Description : Used to open Existing file \n");
        printf("USage : open File_name mode\n");
    }
    else if(strcmp(name,"close") == 0)
    {
        printf("Description : Used to Close opened File \n");
        printf("Usage : close File_name\n");
    }
    else if(strcmp(name,"closeall") == 0)
    {
        printf("Description : Used to close all opened file");
        printf("Usage : closeall\n");
    }
    else if(strcmp(name,"lseek") == 0)
    {
        printf("Description : Used to Change the Fileoffset \n");
        printf("Usage : lseek File_Name ChangeInoffset StartPoint\n");
    }
    else if(strcmp(name,"rm") == 0)
    {
        printf("Description : Usedd to Delete the File");
        printf("Usage : rm File_Name\n");

    }
    else
    {
        printf("ERROR : No manual Entry Available \n");
    }

}

//######################################################################################
//
//	Function Name	: 	DisplayHelp
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	It Display All List / Operations About This Application
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

void DisplayHelp()
{
    printf("ls : To List out all Files \n");
    printf("clear : To clear Console \n");
    printf("open : To Open File \n");
    printf("close : To Close the file \n");
    printf("closeall : To close all opened files \n");
    printf("read : To Read the Contents of the File \n");
    printf("Write : To Write the contents in a file \n");
    printf("exit : To terminate the file system \n");
    printf("stat : To Display information of File using name");
    printf("fstat : To Display the information of file using the file descriptor \n");
    printf("truncate : To Remove all data from a file \n ");
    printf("rm : To Delete the File ");
}

//######################################################################################
//
//	Function Name	: 	GetFDFromName
//	Input			: 	char*
//	Output			: 	Integer
//	Description 	: 	Get File Descriptor Value
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

int GetFDFromName(char *name)
{
    int i =0;
    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName), name) == 0)
            {
               break;
            }
        }
        i++;
    }
    if(i == MAXINODE)
    {
        return -1;
    }

    else
    {
       return i;
    }

}

//######################################################################################
//
//	Function Name	: 	Get_Inode
//	Input			: 	char*
//	Output			: 	PINODE
//	Description 	: 	Return Inode Value Of File
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

PINODE Get_Inode(char *name)
{

    PINODE temp = head;
    int i = 0;

    if(name==NULL)
    {
        return NULL;
    }

    while(temp!=NULL)
    {
        if(strcmp(name,temp->FileName) == 0)
        {
            break;
        }

        temp = temp->next;
    }
        return temp;

}

//######################################################################################
//
//	Function Name	: 	CreateDILB
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	It Creates The DILB When Program Starts
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head; // Insert last function fromlinkedlist


    while(i <= MAXINODE)
    {

        newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount = 0;
        newn->Referencecount = 0;
        newn->FileType = 0;
        newn->FileSize =0;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->InodeNumber = i;

        if(temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp =temp->next;
        }
        i++;

    }
    printf("DILB Created Successfully \n");
}

//######################################################################################
//
//	Function Name	: 	InitialiseSuperBlock
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	Initialize Inode Values
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

void InitialiseSuperBlock()
{

    int i = 0;
    while(i<MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;  // used to set all pointers at null
        i++;
    }
    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;

}

//######################################################################################
//
//	Function Name	: 	CreateFile
//	Input			: 	char*, Integer
//	Output			: 	None
//	Description 	: 	Create New Files
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

int CreateFile(char*name, int permission)
{
    int i= 0;
    PINODE temp = head;

    if((name==NULL)|| (permission == 0)|| (permission > 3))
    {
        return -1;
    }

    if(SUPERBLOCKobj.FreeInode == 0)
    {
        return -2;
    }

    (SUPERBLOCKobj.FreeInode)--;


    if(Get_Inode(name)!=NULL)
    {
        return -3;
    }

    while(temp!= NULL)
    {
        if(temp->FileType == 0)
        {
            break;
        }
        temp=temp->next;
    }
    while(i<MAXINODE)
        {
            if(UFDTArr[i].ptrfiletable == NULL)
            {
              break;
            }
            i++;
        }

        UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

        if(UFDTArr[i].ptrfiletable == NULL)
        {
            return -4;
        }


        UFDTArr[i].ptrfiletable->count = 1;
        UFDTArr[i].ptrfiletable->mode = permission;
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset =0;

        UFDTArr[i].ptrfiletable->ptrinode = temp;

        strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);

        UFDTArr[i].ptrfiletable -> ptrinode -> FileType = REGULAR;
        UFDTArr[i].ptrfiletable -> ptrinode -> Referencecount = 1;
        UFDTArr[i].ptrfiletable -> ptrinode -> LinkCount =1;
        UFDTArr[i].ptrfiletable -> ptrinode -> FileSize = MAXFILESIZE; /// MAXFILESIZE
        UFDTArr[i].ptrfiletable -> ptrinode -> FileActualSize = 0;
        UFDTArr[i].ptrfiletable -> ptrinode -> Permission = permission;
        UFDTArr[i].ptrfiletable -> ptrinode -> Buffer = (char*)malloc(MAXFILESIZE);  /// MAXFILESIZE

        memset(UFDTArr[i].ptrfiletable -> ptrinode -> Buffer, 0, 1024);

        return i ;
}

//######################################################################################
//
//	Function Name	: 	rm_File
//	Input			: 	char*
//	Output			: 	Integer
//	Description 	: 	Remove Created Files
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

 int rm_File(char *name)
 {

     int fd = 0;

      fd = GetFDFromName(name);

      if(fd == -1)
      {
          return -1;

      }

      (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

      if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount==0)
      {
          UFDTArr[fd].ptrfiletable->ptrinode->FileType=0;
          strcpy(UFDTArr[fd].ptrfiletable -> ptrinode -> FileName,"");
          UFDTArr[fd].ptrfiletable -> ptrinode -> Referencecount = 0;
          UFDTArr[fd].ptrfiletable -> ptrinode -> Permission = 0;
          UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize = 0;
          free(UFDTArr[fd].ptrfiletable -> ptrinode -> Buffer);
          free(UFDTArr[fd].ptrfiletable);
      }
      UFDTArr[fd].ptrfiletable = NULL;
      (SUPERBLOCKobj.FreeInode)++;
      printf(" File Deleted Successfully.\n");
 }

 //######################################################################################
//
//	Function Name	: 	ReadFile
//	Input			: 	Integer, char*, Integer
//	Output			: 	Integer
//	Description 	: 	Read Data From File
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

 int ReadFile(int fd, char *arr, int isize)
 {
     int read_size = 0;
     if(UFDTArr[fd].ptrfiletable == NULL)
     {
         return -1;
     }
     if((UFDTArr[fd].ptrfiletable->mode != READ) && ((UFDTArr[fd].ptrfiletable->mode) != READ+WRITE))
     {
         return -2;
     }
     if(UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ+WRITE)
     {
         return-2;
     }
     if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
     {
         return -3;
     }
     if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
     {
         return -4;
     }

     read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)-(UFDTArr[fd].ptrfiletable->readoffset);

     if(read_size < isize)
     {
         strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),read_size);
         UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset+read_size;
     }
     else
     {
         strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),isize);
         (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset)+isize;


     }

     return isize;



 }

 //######################################################################################
//
//	Function Name	: 	WriteFile
//	Input			: 	Integer, char*, Integer
//	Output			: 	Integer
//	Description 	: 	Write Data Into The File
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################
 /// 331 Write file
int WriteFile(int fd, char *arr, int isize)
{
    if(((UFDTArr[fd].ptrfiletable->mode) != WRITE )&& ((UFDTArr[fd].ptrfiletable->mode)!= READ + WRITE))
    {
        return -1;
    }
    if(((UFDTArr[fd].ptrfiletable->ptrinode->Permission)!= WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->Permission) != READ+WRITE))
    {
        return -1;
    }

    if((UFDTArr[fd].ptrfiletable->writeoffset)==2048) /// MAXFILESIZE
    {
        return -2;
    }
    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType)!= REGULAR)
    {
          return -3;
    }
    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);

    (UFDTArr[fd].ptrfiletable->writeoffset)=(UFDTArr[fd].ptrfiletable->writeoffset)+isize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

    return isize;
}

//######################################################################################
//
//	Function Name	: 	OpenFile
//	Input			: 	char*, Integer
//	Output			: 	Integer
//	Description 	: 	Open An Existing File
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if(name == NULL || mode<= 0)
    {
        return -1;
    }
    temp = Get_Inode(name);

    if(temp == NULL)
    {
        return -2;
    }
    if(temp -> Permission < mode)
    {
        return -3;
    }

    while(i<MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            {
                break;
            }
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    if(UFDTArr[i].ptrfiletable==NULL)
    {
        return -1;
    }

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;

    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;

    }
    else if(mode==READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if(mode== WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }

    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->Referencecount)++;

    return i;
    printf("File opened Successfully.\n");

}

//######################################################################################
//
//	Function Name	: 	CloseFileByName
//	Input			: 	Integer
//	Output			: 	None
//	Description 	: 	Close Existing File By By Its File Descriptor
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->Referencecount) = 0;

    printf("File closed Successfully. \n");

}

//######################################################################################
//
//	Function Name	: 	CloseFileByName
//	Input			: 	Char
//	Output			: 	Integer
//	Description 	: 	Close Existing File By Its Name
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

int CloseFileByName(char *name)
{
    int i=0;
    i = GetFDFromName(name);

    if(i==-1)
    {
        return -1;
    }

    if((UFDTArr[i].ptrfiletable -> ptrinode -> Referencecount) == 0)
    {
        return -2;
    }

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->Referencecount) = 0;

    printf("File closed Successfully. \n");

    return 0;
}

//######################################################################################
//
//	Function Name	: 	CloseAllFile
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	Close All Existing Files
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

void CloseAllFile()
{
  int i = 0;
  while(i<MAXINODE)
  {
      if(UFDTArr[i].ptrfiletable != NULL)
      {
         UFDTArr[i].ptrfiletable -> readoffset = 0;
         UFDTArr[i].ptrfiletable -> writeoffset = 0;
         (UFDTArr[i].ptrfiletable -> ptrinode->Referencecount) = 0;
      }
      i++;
  }
  printf("All files are closed successfully. \n");
}

//######################################################################################
//
//	Function Name	: 	LseekFile
//	Input			: 	Integer, Integer, Integer
//	Output			: 	Integer
//	Description 	: 	Write Data Into The File From Perticular Position
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

int LseekFile(int fd, int size, int from)
{
    if((fd<0)||(from>2))
    {
        return -1;
    }

    if(UFDTArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }

    if((UFDTArr[fd].ptrfiletable->mode == READ)||(UFDTArr[fd].ptrfiletable->mode == READ+READ))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset)+size)> UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)

            {
                return -1;
            }
            if(((UFDTArr[fd].ptrfiletable->readoffset)+size)<0)
            {
                return -1;
            }
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset)+size;

        }

        else if(from == START)
        {
          if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
          {
              return -1;
          }
          if(size < 0)
          {
              return -1;
          }
          (UFDTArr[fd].ptrfiletable->readoffset) = size;
        }

        else if(from == END)
        {
            if(((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ size) > MAXFILESIZE) /// MAXFILESIZE
            {
                return -1;
            }
            if(((UFDTArr[fd].ptrfiletable->readoffset)+size)<0)
            {
                return -1;
            }
            (UFDTArr[fd].ptrfiletable->readoffset)  = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size;

        }
    }
        else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
        {
            if(from == CURRENT)
            {
                if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)>MAXFILESIZE) /// MAXFILESIZE
                {
                    return -1;
                }
                if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)<0)
                {
                    return -1;
                }
                if(((UFDTArr[fd].ptrfiletable->writeoffset)>size)>(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                 {


                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable->writeoffset) + size;

                (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset)+size;

                 }
            }

            else if(from == START)
            {
              if(size>MAXFILESIZE) /// MAXFILESIZE
              {
                  return -1;
              }

              if(size<0)
              {
                  return -1;
              }

              if(size>(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
               {
                   (UFDTArr[fd].ptrfiletable->writeoffset)=size;
                   (UFDTArr[fd].ptrfiletable->writeoffset) = size;
               }
            }

            else if(from == END)
            {
              if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size>MAXFILESIZE) /// MAXFILESIZE
              {
                  return -1;
              }
              if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)<0)
              {
                  return -1;
              }

              (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size;

            }
        }
        printf("Successfully Changed \n");
    }


//######################################################################################
//
//	Function Name	: 	ls_file
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	List Out All Existing Files Name
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

    void ls_file()
    {
        int i = 0;
        PINODE temp = head;

        if(SUPERBLOCKobj.FreeInode == MAXINODE)
        {
            printf("ERROR : There are No Files");
            return;
        }
        printf("\n File Name\tInode number\tFile Size\tLink Count\n");
        printf("------------------------------------------------------------------\n");
        while(temp != NULL)
        {
            if (temp->FileType !=0)
            {
                printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileActualSize,temp->LinkCount);

            }
            temp=temp->next;
        }
        printf("-------------------------------------------------------------------\n");

    }


//######################################################################################
//
//	Function Name	: 	fstat_file
//	Input			: 	Integer
//	Output			: 	Integer
//	Description 	: 	Display Statistical Information Of The File By Using File Descriptor
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################
    int fstat_file(int fd)
    {
        PINODE temp = head;

        int i = 0;

        if(fd < 0)
        {
            return -1;
        }
        if(UFDTArr[fd].ptrfiletable == NULL)
        {
            return -2;
        }

        temp = UFDTArr[fd].ptrfiletable->ptrinode;

        printf("\n-------------Statistical Information about file---------\n");
        printf("File name: %s\n",temp -> FileName);
        printf("Inode Number: %d\n",temp -> InodeNumber);
        printf("File Size: %d\n",temp -> FileSize);
        printf("Actual File Size: %d\n",temp -> FileActualSize);
        printf("Link Count: %d\n",temp -> LinkCount);
        printf("Reference Count: %d\n",temp -> Referencecount);

        if(temp->Permission == 1)
        {
            printf("File Permission : Read Only\n ");
        }
        else if(temp->Permission == 2)
        {
            printf("File Permission : Write Only\n ");
        }
        else if(temp->Permission == 3)
        {
            printf("File Permission : Read & Write\n ");
        }
        printf("------------------------------------------------------\n");
        return 0;
    }

//######################################################################################
//
//	Function Name	: 	stat_file
//	Input			: 	Char*
//	Output			: 	Integer
//	Description 	: 	Display Statistical Information Of The File By Using File Name
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

    int stat_file(char *name)
    {
        PINODE temp = head;
        int i =0;

        if (name == NULL)
        {
            return -1;
        }
        while(temp != NULL)
        {
            if (strcmp(name,temp->FileName)==0)
            {
                break;
            }
            temp = temp->next;

        }

        if(temp == NULL)
        {
            return -2;
        }

        printf("\n-------------Statistical Information about file---------\n");
        printf("File name: %s\n",temp->FileName);
        printf("Inode Number: %d\n",temp->InodeNumber);
        printf("File Size: %d\n",temp->FileSize);
        printf("Actual File Size: %d\n",temp->FileActualSize);
        printf("Link Count: %d\n",temp->LinkCount);
        printf("Reference Count: %d\n",temp->Referencecount);

        if(temp->Permission == 1)
        {
            printf("File Permission : Read Only\n ");
        }
        else if(temp->Permission == 2)
        {
            printf("File Permission : Write Only\n ");
        }
        else if(temp->Permission == 3)
        {
            printf("File Permission : Read & Write\n ");
        }

        printf("------------------------------------------------------\n");

        return 0;

    }

//######################################################################################
//
//	Function Name	: 	truncate_File
//	Input			: 	Char*
//	Output			: 	Integer
//	Description 	: 	Delete All Data From The File
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

    int truncate_File(char *name)
    {
        int fd = GetFDFromName(name);

        if(fd==-1)
        {
            return -1;
        }

        memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,MAXFILESIZE);
        UFDTArr[fd].ptrfiletable->readoffset = 0;
        UFDTArr[fd].ptrfiletable->writeoffset = 0;
        UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;

        printf("Data Successfully Removed \n");
    }

//######################################################################################
//
//	Function Name	: 	Backup
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	Take Backup Of All Created Files Into Hard-Disk
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

    void backup()
{
    PINODE temp = head;

    int fd = 0;

    while(temp != NULL)
    {
        if(temp -> FileType != 0)
        {
            fd = creat(temp -> FileName, 0777);
            write(fd, temp -> Buffer, temp -> FileActualSize);
        }
        temp = temp -> next;
    }
    printf("Successfully Get The Backup Of All Created Files...\n");
}


//######################################################################################
//
//	Function Name	: 	main
//	Input			: 	None
//	Output			: 	Integer
//	Description 	: 	Entry Point Function
//	Author			: 	Vaibhav Konnur
//	Date			:	27 january 2023
//
//######################################################################################

int main()
{
    char *ptr = NULL;

	int ret = 0, fd = 0, count = 0;
	char command[4][80], str[80], arr[MAXFILESIZE];

	InitialiseSuperBlock();
	CreateDILB();

	while(1)
	{
		fflush(stdin);
		strcpy(str, "");

		printf("\nCustomised VFS : > ");

		fgets(str, 80, stdin); // scanf("%[^'\n']s",str);

		count = sscanf(str,"%s %s %s %s", command[0], command[1], command[2], command[3]);

		if(count == 1)
		{
			if(strcmp(command[0], "ls") == 0)
			{
				ls_file();
			}

			else if(strcmp(command[0], "closeall") == 0)
			{
				CloseAllFile();
				continue;
			}

			else if(strcmp(command[0], "clear") == 0)
			{
				system("cls");
				continue;
			}

			else if(strcmp(command[0], "help") == 0)
			{
				DisplayHelp();
				continue;
			}

            		else if(strcmp(command[0], "backup") == 0)
            		{
                		backup();
                		continue;
            		}

			else if(strcmp(command[0], "exit") == 0)
			{
				printf("Terminating the Customised Virtual File System\n");
				break;
			}

			else
			{
				printf("\nERROR : Command not found !!!\n");
				continue;
			}
		}

		else if(count == 2)
		{
			if(strcmp(command[0], "stat") == 0)
			{
				ret = stat_file(command[1]);
				if(ret == -1)
                		{
                   	 		printf("ERROR : Incorrect parameters\n");
                		}

				if(ret == -2)
                		{
                    			printf("ERROR : There is no such file\n");
                		}
                		continue;
			}

			else if(strcmp(command[0], "fstat") == 0) //fstat 0
			{
				ret = fstat_file(atoi(command[1]));

				if(ret == -1)
                		{
                   	 		printf("ERROR : Incorrect parameters\n");
                		}

				if(ret == -2)
                		{
                    			printf("ERROR : There is no such file\n");
                		}
                		continue;
			}

			else if(strcmp(command[0], "close") == 0)
			{
				ret = CloseFileByName(command[1]);

				if(ret == -1)
                		{
                    			printf("ERROR : There is no such file\n");
                		}

                		if (ret == -2)
                		{
                    			printf("The File Is Already Closed\n");
                		}
                		continue;
			}

			else if(strcmp(command[0], "rm") == 0)
			{
				ret = rm_File(command[1]);

				if(ret == -1)
                		{
                    			printf("ERROR : There is no such file\n");
					continue;
                		}
			}

			else if(strcmp(command[0], "man") == 0)
			{
				man(command[1]);
			}

			else if(strcmp(command[0], "write") == 0)
			{
				fd = GetFDFromName(command[1]);

                		if(fd == -1)
				{
					printf("Error : Incorrect parameter\n");
					continue;
				}

                if(UFDTArr[fd].ptrfiletable -> ptrinode -> Referencecount == 0)
                {
                    printf("ERROR : File is Not Opened\n");
                }

                else
                {
                    printf("Enter the data : \n");
				    scanf("%[^\n]", arr);
                }
				//fflush(stdin); // empty input buffer

				ret = strlen(arr);

				if(ret == 0)
				{
					printf("Error : Incorrect parameter\n");
					continue;
				}

				ret = WriteFile(fd,arr,ret); // 0, Adress, 4

				if(ret == -1)
                		{
                    			printf("ERROR : Permission denied\n");
                		}

				if(ret == -2)
                		{
                    			printf("ERROR : There is no sufficient memory to write\n");
                		}

				if(ret == -3)
                		{
                    			printf("ERROR : It is not regular file\n");
                		}

				if(ret == -4)
				{
					printf("ERROR:There is no sufficient memory Available\n");
				}

                if(ret > 0)
				{
					printf("Sucessfully : %d bytes written\n", ret);
				}
			}


			else if(strcmp(command[0], "truncate") == 0)
			{
				ret = truncate_File(command[1]);

				if(ret == -1)
                		{
                    			printf("Error : Incorrect parameter\n");
                		}
			}

			else
			{
				printf("\nERROR : Command not found !!!\n");
			}
            		continue;
		}

		else if(count == 3)
		{
			if(strcmp(command[0], "create") == 0)
			{
				ret = CreateFile(command[1], atoi(command[2])); // ASCII to Integer

				if(ret >= 0)
				        {
                    			printf("File is successfully created with file descriptor : %d\n", ret);
                		}

				if(ret == -1)
                		{
                    			printf("ERROR : Incorrect parameters\n");
                		}

				if(ret == -2)
                		{
                    			printf("ERROR : There is no inodes\n");
                		}

				if(ret == -3)
                		{
                    			printf("ERROR : File already exists\n");
                		}

				if(ret == -4)
                		{
                    			printf("ERROR : Memory allocation failure\n");
                		}
                		continue;
			}

			else if(strcmp(command[0], "open") == 0)
			{
				ret = OpenFile(command[1], atoi(command[2]));

				if(ret >= 0)
                		{
                    			printf("File is successfully opened with file descriptor : %d\n", ret);
                		}

				if(ret == -1)
                		{
                    			printf("ERROR : Incorrect parameters\n");
                		}

				if(ret == -2)
                		{
                   	 		printf("ERROR : File not present\n");
                		}

				if(ret == -3)
                		{
                    			printf("ERROR : Permission denied\n");
                		}
                		continue;
			}

			else if(strcmp(command[0], "read") == 0)
			{
				fd = GetFDFromName(command[1]);

				if(fd == -1)
				{
					printf("Error : Incorrect parameter\n");
					continue;
				}

				ptr = (char *)malloc(sizeof(atoi(command[2])) + 1);

				if(ptr == NULL)
				{
					printf("Error : Memory allocation failure\n");
					continue;
				}

				ret = ReadFile(fd, ptr, atoi(command[2]));

                if(ret == -1)
                {
                    printf("ERROR : File not existing\n");
                }

				if(ret == -2)
                		{
                    			printf("ERROR : Permission denied\n");
                		}

				if(ret == -3)
                		{
                    			printf("ERROR : Reached at end of file\n");
                		}

				if(ret == -4)
                		{
                    			printf("ERROR : It is not regular file\n");
                		}

                if (ret == -5)
                		{
                    			printf("ERROR : File is not opened\n");
                		}

				if(ret == 0)
                		{
                    			printf("ERROR : File empty\n");
                		}

				if(ret > 0)
				{
					write(2, ptr, ret);  // 0 for stdin, 1 for stdout
				}
				continue;
			}

			else
			{
				printf("\nERROR : Command not found !!!\n");
				continue;
			}
		}

		else if(count == 4)
		{
			if(strcmp(command[0], "lseek") == 0)
			{
				fd = GetFDFromName(command[1]);

				if(fd == -1)
				{
					printf("Error : Incorrect parameter\n");
					continue;
				}

				ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));

                		if(ret == -1)
				{
					printf("ERROR : Unable to perform lseek\n");
				}

                		if (ret == -2)
                		{
                    			printf("ERROR : File is not opened\n");
                		}
			}

			else
			{
				printf("\nERROR : Command not found !!!\n");
				continue;
			}
		}

		else
		{
			printf("\nERROR : Command not found !!!\n");
			continue;
		}
	}
    return 0;
}
