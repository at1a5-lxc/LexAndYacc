#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <map>
#include <cstdlib>
#include <queue>
#include "mainHeader.h"
#include <cstddef>
#define debugprint printf
using namespace std;

ifstream ifile;
ofstream ofile;

class Item{
public:
	string left;//产生式左部
	vector<string> right;//产生式右部
	int dotPosition;//记录点的位置
	string lookaheadSymbol;//可规约预测符号集合 每个item设置为一个
	Item(){dotPosition=0;}
	bool Move()
	{
		if(GetCurrentSymbol()!=END)//防止溢出
		{
			dotPosition++;//移动点的位置
			return true;//移动成功  返回true
		}
		else
			return false;//已经属于最尾部  返回false
	}
	string GetCurrentSymbol()
	{
		if((uint32_t)dotPosition<right.size())
			return right[dotPosition];
		return END;//定义的终止符
	}
	string GetNextSymbol()
	{
		if((uint32_t)dotPosition+1<right.size())
			return right[dotPosition+1];
		return END;//定义的终止符
	}
	void printItem()
	{
		printf("%s ---> ",left.c_str());
		for(uint32_t i=0;i<right.size();i++)
		{
			if(dotPosition==i)
				printf(".");
			printf("%s ",right[i].c_str());
		}
		if(dotPosition==right.size())
			printf(". ");
		printf(",%s\n",lookaheadSymbol.c_str());
	}
};

struct PDAedge{//PDA边
		int nextState;//指向下一状态
		string symbol;
	};

struct PDAstate{//PDA状态
	int id;
	vector<Item> itemSet;

	vector<PDAedge> edges;
	bool ContainEdge(PDAedge input)
	{
		for(u32 i=0;i<edges.size();i++)
		{
			if(edges[i].nextState==input.nextState&&edges[i].symbol==input.symbol)
				return true;
		}
		return false;
	}
	void printState()
	{
		printf("StateID:%d\n",id);
		for(uint32_t i=0;i<itemSet.size();i++)
		{
				cout<<"\t";
				itemSet[i].printItem();
		}
		for(uint32_t i=0;i<edges.size();i++)
		{
			printf("(%d) ---%s---> (%d)\n",id,edges[i].symbol.c_str(),edges[i].nextState);
		}
		printf("\n");
	}
};

struct PDA{
	vector<PDAstate> states;
	void printState()
	{
		for(u32 i=0;i<states.size();i++)
			states[i].printState();
	}
};


set <string> Terminal;//终结符集
set <string> NonTerminal;//非终结符集
//set <Item>	 Producers;//产生式集
vector <Item>	 Producers;//产生式集  使用set报错？？？
PDA globalPDA;


struct Op{
	string name;
	int priority;
	string type;
};
vector<Op> Operators;

set<string> First(string input)
{
	set<string> ret;
	if(input==END)return ret;
	if(Terminal.count(input)!=0)
	{
		ret.insert(input);
		return ret;
	}
	for(auto i=Producers.begin();i!=Producers.end();i++)
	{
		if(i->left==input)
		{
			if(Terminal.count(i->right[0])!=0)
				ret.insert(i->right[0]);//如果当前input的右部第一个是终结符 那么加入结果集
			else
			{
				set<string> res=First(i->right[0]);//递归查询first
				for(auto j=res.begin();j!=res.end();j++)
				{
					ret.insert(*j);//将新的first合并到结果集中
				}
			}
		}
	}
	return ret;
}

set<string> FirstDouble(string input0,string input1)
{
	set<string> ret;
	string input="";
	if(input0==END)
		input=input1;
	else
		input=input0;
	ret = First(input);
	return ret;
}

bool ItemContained(vector<Item> input0,Item input1)
{
	for(uint32_t i=0;i<input0.size();i++)
	{
		if(input0[i].left==input1.left&&input0[i].dotPosition==input1.dotPosition&&input0[i].right.size()==input1.right.size()&&input0[i].lookaheadSymbol==input1.lookaheadSymbol)
		{
			bool flag=true;
			for(uint32_t j=0;j<input0[i].right.size();j++)
			{
				if(input0[i].right[j]!=input1.right[j]){flag=false;break;}
			}
			if(flag==true)
				return true;
		}
	}
	return false;
}

bool StateEqual(PDAstate A,PDAstate B)//判断两个状态是否相等，这个时间复杂度。。。
{
	if(A.itemSet.size()!=B.itemSet.size())
		return false;//如果大小不相同  两个状态肯定不相同
	for(int i=0;i<B.itemSet.size();i++)
	{
		if(!ItemContained(A.itemSet,B.itemSet[i]))//如果任何一个B中的状态不存在于A中那么肯定不相等
			return false;
	}
	return true;
}
int PDAstateContained(PDA input0,PDAstate input1)//判断一个PDAstate是否出现在PDA中
{
	for(int i=0;i<input0.states.size();i++)
	{
		//判断每个状态是否与当前这个状态相同
		if(StateEqual(input0.states[i],input1))
			return i;//只要input0中有一个状态与input1状态相等，那么input1就包含于input0中
	}
	return -1;
}
void Closure(PDAstate& input)
{//！！注意是要向I中不停迭代，需要检测前后两次迭代是否再次产生新的产生式加入其中
	vector<Item> I=input.itemSet;
	int lastSize;
	do
	{
		lastSize=I.size();//记录I的大小 用作迭代
		string A;
		string alpha,B,beta;
		string a;
		for(uint32_t itemIndex=0;itemIndex<I.size();itemIndex++)//(auto eachItem=I.begin();eachItem!=I.end();eachItem++)
		{
			Item eachItem=I[itemIndex];
			B=eachItem.GetCurrentSymbol();//B是当前项目的点所指，如果为END表示为空
			beta=eachItem.GetNextSymbol();//beta是B后面的符号
			a=eachItem.lookaheadSymbol;
			//接下来找到所有由B引出的产生式
			if(B!=END)
			{
				//遍历所有由B作为左部的产生式
				for(auto eachp=Producers.begin();eachp!=Producers.end();eachp++)
				{
					if(eachp->left==B)
					{
						set<string> firstBetAlp=FirstDouble(beta,a);
						//printf("[+]Size of first(%s,%s)=%d\n",beta.c_str(),a.c_str(),firstBetAlp.size());
						for(auto eachfirst=firstBetAlp.begin();eachfirst!=firstBetAlp.end();eachfirst++)
						{
							Item toAddItem;
							toAddItem.left=eachp->left;
							toAddItem.right=eachp->right;
							toAddItem.dotPosition=0;
							toAddItem.lookaheadSymbol=*eachfirst;
							//将新产生的产生式加入到集合I中
							//cout<<"Add new item to I:";
							//toAddItem.printItem();
							//判断当前的I中是否包含toAddItem
							if(!ItemContained(I,toAddItem))
								I.push_back(toAddItem);
						}
					}
				}
			}
		}
	}
	while(I.size()!=lastSize);//如果此次的大小和上次大小相同 说明不再产生新的produce 迭代结束
	input.itemSet=I;
}
PDAstate GOTO(PDAstate I,string X)
{
	PDAstate J;//将J初始化为空集
	for(uint32_t itemIndex=0;itemIndex<I.itemSet.size();itemIndex++)//(auto eachItem=I.begin();eachItem!=I.end();eachItem++)
	{//对于I中的每个项，[A->alpha. X beta,a]
		//将项[A->alpha X .beta,a]加入到J中
		Item eachItem=I.itemSet[itemIndex];
		if(eachItem.GetCurrentSymbol()==X)
		{
			
			eachItem.Move();//如果当前项是X 那么移动点到后面
			J.itemSet.push_back(eachItem);//将其加入到J中
		}
	}
	Closure(J);//对J进行闭包处理
	//debugprint("[=]");
	return J;
}


void ParseYaccFile()
{
	string srcFile = "D:\\minic.y";//Yacc文件
	
	ifile.open(srcFile.c_str(), ios::in);
	ofile.open("D:\\generatedYacc.h", ios::out);
	if (!ifile.is_open()) {
		cerr << "[-]Error occured when open file!" << endl;
		exit(1);
	}
	char c=0x00;
	while(c!='%'&&!ifile.eof())
	{
		//一直读取到第一个%
		c = ifile.get();
	}
	if(c=='%')//读取到了第一个%
	{
		if(!(!ifile.eof()&&ifile.get()=='{'))
			cerr<<"[-]Wrong format!"<<endl;
	}
	bool endState=false;
	char nextC=0x00;
	while(!endState)
	{
		c=ifile.get();
		if(c!='%')
		{
			ofile.put(c);
		}
		else
		{
			nextC=ifile.get();
			if(nextC!='}')
				ofile.put(nextC);
			else
				endState=true;
		}
	}
	std::cout<<"[+]Definition segment completed!\n";
	//开始解析终结符和运算符
	string lineStr;
	char tempLineCStr[500];
	endState=false;
	int currentPriority=0;
	while(!endState)
	{
		ifile.getline(tempLineCStr,500);
		lineStr=tempLineCStr;
		//删除注释部分
		int annoteIndex=lineStr.find("/*");
		if(annoteIndex!=-1)
			lineStr=lineStr.substr(0,annoteIndex);
		//默认每一行的有效标识符从行首开始
		if(lineStr.find("%%")!=string::npos)
		{
			//找到%%标识符 说明定义段结束 跳出
			endState=true;
		}
		else
		{
			string markWord="";//用来存储行首的标识符
			char toSplit[200];
			strcpy(toSplit,lineStr.c_str());
			char *toke;
			char *nextToke;
			if(lineStr[0]=='%')//如果不是空行 则进行处理
			{//toke存储的是行首的标记符 有%token %type %nonassoc %left
				toke=strtok_s(toSplit,"\t \n",&nextToke);
				//std::cout<<"[toke]"<<toke<<"[+]\n";
				if(strcmp(toke,"%token")==0)//此行记录的是token
				{
					while(toke!=0)
					{
						toke=strtok_s(0,"\t \n",&nextToke);
						if(toke!=0)
						{//存储每一个terminal到集合中
							string tempToken=toke;
							Terminal.insert(tempToken);
						}
					}
				}
				else if(strcmp(toke,"%nonassoc")==0)//此行记录的是nonassoc
				{
					while(toke!=0)
					{
						toke=strtok_s(0,"\t \n",&nextToke);
						if(toke!=0)
						{//存储每一个nonassoc op到Operators中
							Op newOp;
							newOp.name=toke;
							newOp.priority=currentPriority;
							newOp.type="nonassoc";
							Operators.push_back(newOp);
						}
					}
					currentPriority++;
				}
				else if(strcmp(toke,"%left")==0)//此行记录的是nonassoc
				{
					while(toke!=0)
					{
						toke=strtok_s(0,"\t \n",&nextToke);
						if(toke!=0)
						{//存储每一个nonassoc op到Operators中
							Op newOp;
							newOp.name=toke;
							newOp.priority=currentPriority;
							newOp.type="left";
							Operators.push_back(newOp);
						}
					}
					currentPriority++;
				}
				else if(strcmp(toke,"%type")==0)//此行记录的是type
				{
				}
				else if(strcmp(toke,"%union")==0)//此行记录的是type
				{
				}
			}
		}
	}
	//开始解析规则段
	endState=false;
	bool newLeftMark=true;
	string tempLeft="";
	vector<string> tempRight;
	while(!endState)
	{
		ifile.getline(tempLineCStr,500);
		lineStr=tempLineCStr;
		//删除注释部分
		int annoteIndex=lineStr.find("/*");
		if(annoteIndex!=-1)
			lineStr=lineStr.substr(0,annoteIndex);
		lineStr=lineStr+'\0';
		//默认每一行的有效标识符从行首开始
		if(lineStr.find("%%")!=-1)
		{
			//找到%%标识符 说明规则段结束 跳出
			endState=true;
		}
		else
		{
			char toSplit[200];
			strcpy(toSplit,lineStr.c_str());
			char *toke=NULL;
			char *nextToke=NULL;
			//查找当前是否包含：  如果有 说明出现了新的左部
			if(lineStr.find(":")!=-1)
			{
				newLeftMark=true;//记录当前遇到了新的左部
			}
			//debugprint("%s\n",lineStr.c_str());
			toke=strtok_s(toSplit,"\t \n",&nextToke);
			while(toke!=NULL)
			{
				if(newLeftMark)//如果我遇到了leftMark 很显然toke是左部
				{
					newLeftMark=false;
					tempLeft=toke;
					if(Terminal.count(string(toke))==0)//如果当前的toke不是终结符 那么一定是非终结符
						NonTerminal.insert(toke);
					toke=strtok_s(NULL,"\t \n",&nextToke);
					continue;
				}
				//否则toke是右部或者 ：
				else if(string(toke)==":")
				{
					//遇到 ： 或者 | 或者 ；直接快进
					toke=strtok_s(NULL,"\t \n",&nextToke);
					continue;
				}
				else if(string(toke)=="|"||string(toke)==";")
				{//此时上一个产生式的右部已经采集完毕
					toke=strtok_s(NULL,"\t \n",&nextToke);
					Item tempProducer;
					tempProducer.dotPosition=0;
					tempProducer.left=tempLeft;
					for(uint32_t i=0;i<tempRight.size();i++)
					{
						tempProducer.right.push_back(tempRight[i]);
					}
					Producers.push_back(tempProducer);//将刚发现的产生式加入到Producer中
					tempRight.clear();//清除上一次的右部
				}
				else//当前遇到的是右部
				{
					if(string(toke).find("{")==-1)//排除语义动作SemantAction
					{
						tempRight.push_back(string(toke));
						if(Terminal.count(string(toke))==0)//如果当前的toke不是终结符 那么一定是非终结符
							NonTerminal.insert(toke);
					}
					toke=strtok_s(NULL,"\t \n",&nextToke);
				}
			}
		}
	}

	//开始拷贝子程序段

	//输出部分结果
	std::cout<<"[+]Print Terminal Set\n";
	for(set<string>::iterator it=Terminal.begin();it!=Terminal.end();it++)
	{
		//debugprint("%s\t",*it->c_str());
		cout<<"<"<<*it<<">\t";
	}
	cout<<endl;
	std::cout<<"[+]Print NonTerminal Set\n";
	for(set<string>::iterator it=NonTerminal.begin();it!=NonTerminal.end();it++)
	{
		cout<<"<"<<*it<<">\t";
	}
	cout<<endl;
	cout<<"[+]Print Producers\n";
	for(uint32_t i=0;i<Producers.size();i++)
	{
		cout<<"("<<Producers[i].left<<") ----> ";
		for(uint32_t j=0;j<Producers[i].right.size();j++)
			cout<<"("<<Producers[i].right[j]<<") ";
		cout<<endl;
	}
	std::cout<<"\n[+]Print Operator\n";
	for(uint32_t i=0;i<Operators.size();i++)
	{
		std::cout<<Operators[i].name<<"\t"<<Operators[i].type<<"\t"<<Operators[i].priority<<"\t"<<endl;
	}
	ifile.close();
	ofile.close();
}
void GeneratePDA()
{
	PDA myPDA;
	//默认Producer中存放的第0个产生式就是初始产生式 小心bug
	PDAstate S0;//设置初始状态
	Item item0;
	int globalStateId=0;
	Terminal.insert("$");//向终结符中插入结束符
	//设置初始状态
	S0.id=globalStateId++;
	if(Producers.size()==0)return;//如果当前灭有产生式 那就不玩了
	Producers[0].lookaheadSymbol="$";//首个符号加入$作为终结表示
	S0.itemSet.push_back(Producers[0]);//将第0个产生式加入初始状态中
	Closure(S0);
	myPDA.states.push_back(S0);
	debugprint("[+]***************PDA BEGIN*****************\n");

	uint32_t lastSize=myPDA.states.size();
	do
	{
		lastSize=myPDA.states.size();
		for(int i=0;i<myPDA.states.size();i++)
		{
			//对于每一个状态，实施GOTO X 行为
			PDAstate newPDAstate;
			//下面是对当前的状态施行基于终结符和非终结符的GOTO操作
			//如果 GOTO的结果非空并且目前的状态集中不含有 那么加入作为一个新状态
			for(auto ter=Terminal.begin();ter!=Terminal.end();ter++)
			{
				newPDAstate=GOTO(myPDA.states[i],*ter);
				if(newPDAstate.itemSet.size()!=0)
				{
					int stateId=PDAstateContained(myPDA,newPDAstate);
					if(stateId==-1)//找不到这个状态，说明是新状态
					{
						newPDAstate.id=globalStateId++;
						myPDA.states.push_back(newPDAstate);
						//为这个新的状态创建边
						PDAedge newEdge;
						newEdge.nextState=newPDAstate.id;//新边指向的状态的号码
						newEdge.symbol=*ter;//新边上的符号标记
						myPDA.states[i].edges.push_back(newEdge);
					}
					else
					{
						PDAedge newEdge;
						newEdge.nextState=stateId;//新边指向的状态的号码
						newEdge.symbol=*ter;//新边上的符号标记
						if(myPDA.states[i].ContainEdge(newEdge)==false)//如果不包含这条边 加进去
							myPDA.states[i].edges.push_back(newEdge);
					}
				}
			}
			for(auto ter=NonTerminal.begin();ter!=NonTerminal.end();ter++)
			{
				newPDAstate=GOTO(myPDA.states[i],*ter);
				if(newPDAstate.itemSet.size()!=0)
				{
					int stateId=PDAstateContained(myPDA,newPDAstate);
					if(stateId==-1)//找不到这个状态，说明是新状态
					{
						newPDAstate.id=globalStateId++;
						myPDA.states.push_back(newPDAstate);
						//为这个新的状态创建边
						PDAedge newEdge;
						newEdge.nextState=newPDAstate.id;//新边指向的状态的号码
						newEdge.symbol=*ter;//新边上的符号标记
						myPDA.states[i].edges.push_back(newEdge);
					}
					else
					{
						PDAedge newEdge;
						newEdge.nextState=stateId;//新边指向的状态的号码
						newEdge.symbol=*ter;//新边上的符号标记
						if(myPDA.states[i].ContainEdge(newEdge)==false)//如果不包含这条边 加进去
							myPDA.states[i].edges.push_back(newEdge);
					}
				}
			}
		}
	}
	while(myPDA.states.size()!=lastSize);//重复迭代一直到PDA的大小不再变化
	myPDA.printState();
	debugprint("\n[+]Generate PDA test finish\n");
}

void main()
{
	//Step1. 读取yacc文件
	ParseYaccFile();
	//Step2. 生成下推自动机
	GeneratePDA();
	//Step3. 
	std::cout<<"Done\n";
}


void main_()
{
	string lineStr="abc";
	lineStr=("ff");
	cout<<"[]"<<lineStr<<"[]"<<endl;
	/*
	string firstEle="S";
	set<string> res=First(firstEle);
	printf("First(%s) = ",firstEle.c_str());
	for(auto i=res.begin();i!=res.end();i++)
		cout<<*i<<"\t";
	cout<<endl;
	*/	
	/*
					else
					{
						debugprint("\n[?]new state but already exsitd\n");
						newPDAstate.printState();
					}
					*/
}