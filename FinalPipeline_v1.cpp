#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem;
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable;
    bool        nop;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem;
    bool        wrt_enable;
    bool        nop;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

bitset<32> signextend (bitset<16> imm)
{
    string sestring;
    if (imm[15]==0){
        sestring = "0000000000000000"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    else{
        sestring = "1111111111111111"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    return (bitset<32> (sestring));

}

unsigned long shiftbits(bitset<32> inst, int start)
{
    unsigned long ulonginst;
    return ((inst.to_ulong())>>start);

}



class RF
{
    public:
        bitset<32> Reg_data;
     	RF()
    	{
			Registers.resize(32);
			Registers[0] = bitset<32> (0);
        }

        bitset<32> readRF(bitset<5> Reg_addr)
        {
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }

        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }

		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();
		}

	private:
		vector<bitset<32> >Registers;
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {
			IMem.resize(MemSize);
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{
					IMem[i] = bitset<8>(line);
					i++;
				}
			}
            else cout<<"Unable to open file";
			imem.close();
		}

		bitset<32> readInstr(bitset<32> ReadAddress)
		{
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;
		}

    private:
        vector<bitset<8> > IMem;
};

class DataMem
{
    public:
        bitset<32> ReadData;
        DataMem()
        {
            DMem.resize(MemSize);
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();
        }

        bitset<32> readDataMem(bitset<32> Address)
        {
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;
		}

        void writeDataMem(bitset<32> Address, bitset<32> WriteData)
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));
        }

        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {
                    dmemout << DMem[j]<<endl;
                }

            }
            else cout<<"Unable to open file";
            dmemout.close();
        }

    private:
		vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl;

        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl;

        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl;
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl;
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl;
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl;
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl;
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl;
    }
    else cout<<"Unable to open file";
    printstate.close();
}


int main()
{

    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;

	stateStruct state;
	stateStruct newState;

	state.IF.nop = false;
	state.ID.nop = true;
	state.EX.nop = true;
	state.MEM.nop = true;
	state.WB.nop = true;

    state.WB.Rs = 0;
    state.WB.Rt = 0;
    state.WB.Wrt_data = 0;
    state.WB.wrt_enable = false;
    state.WB.Wrt_reg_addr = 0;

    state.MEM.ALUresult = 0;
    state.MEM.rd_mem = 0;
    state.MEM.Rs = 0;
    state.MEM.Rt = 0;
    state.MEM.Store_data = 0;
    state.MEM.wrt_enable = 0;
    state.MEM.wrt_mem = 0;
    state.MEM.Wrt_reg_addr = 0;

    state.ID.Instr = 0;

    state.EX.alu_op = 0;
    state.EX.Imm = 0;
    state.EX.is_I_type = false;
    state.EX.rd_mem = false;
    state.EX.Read_data1 = 0;
    state.EX.Read_data2 = 0;
    state.EX.Rs = 0;
    state.EX.Rt = 0;
    state.EX.wrt_enable = 0;
    state.EX.wrt_mem = 0;
    state.EX.Wrt_reg_addr = 0;


    newState.WB.Rs = 0;
    newState.WB.Rt = 0;
    newState.WB.Wrt_data = 0;
    newState.WB.wrt_enable = false;
    newState.WB.Wrt_reg_addr = 0;
    newState.WB.nop = true;

    newState.MEM.ALUresult = 0;
    newState.MEM.rd_mem = 0;
    newState.MEM.Rs = 0;
    newState.MEM.Rt = 0;
    newState.MEM.Store_data = 0;
    newState.MEM.wrt_enable = 0;
    newState.MEM.wrt_mem = 0;
    newState.MEM.Wrt_reg_addr = 0;
    newState.MEM.nop = true;

    newState.ID.Instr = 0;
    newState.ID.nop = true;

    newState.EX.alu_op = 0;
    newState.EX.Imm = 0;
    newState.EX.is_I_type = false;
    newState.EX.rd_mem = false;
    newState.EX.Read_data1 = 0;
    newState.EX.Read_data2 = 0;
    newState.EX.Rs = 0;
    newState.EX.Rt = 0;
    newState.EX.wrt_enable = 0;
    newState.EX.wrt_mem = 0;
    newState.EX.Wrt_reg_addr = 0;
    newState.EX.nop = true;


    bitset<32> ALUin1;
    bitset<32> ALUin2;

    bitset<6> opcode;
    bitset<1> RType;
    bitset<1> IType;
    bitset<1> JType;
    bitset<1> IsBranch;
    bitset<1> IsLoad;
    bitset<1> IsStore;
    bitset<1> WrtEnable;

    bitset<32> braddr;


    bitset<6> funct;
    bitset<32> signext;
	// instruction
    state.IF.PC = 0;

    bool stall = false;
    bool IsTaken = false;

    int MAX = 100;
    int clk = 1;



      while(1)
        //for (clk=1; clk<MAX; clk++)
        {

		/* --------------------- WB stage --------------------- */
		/*r4 = M4(r3);*/
		if(state.WB.nop)
        {
          //  state.WB.Wrt_reg_addr = 0;
          //  state.WB.Wrt_data = 0;
          //  state.WB.wrt_enable = 0;
        }
        else
        {


			if(state.WB.wrt_enable)
            {myRF.writeRF(state.WB.Wrt_reg_addr,state.WB.Wrt_data);}
        }


        /* --------------------- MEM stage ---------------------*/
		/*r4 = M4(r3);*/
		if(state.MEM.nop)
        {
 		  newState.WB.nop = true;

        }
		else
		{
            if(state.MEM.wrt_mem)
		{myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);}

		 newState.WB.Rs = state.MEM.Rs;
		 newState.WB.Rt = state.MEM.Rt;
		 newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
		 newState.WB.Wrt_data = (state.MEM.rd_mem)?myDataMem.readDataMem(state.MEM.ALUresult):state.MEM.ALUresult;
		 newState.WB.wrt_enable = state.MEM.wrt_enable;

		 newState.WB.nop = state.MEM.nop;
		}

        /* --------------------- EX stage --------------------- */
        /*r3 = M3(r2);*/
        if(state.EX.nop)
        {
            newState.MEM.nop = true;
        }
		else
        {

        if (!state.WB.nop && state.WB.wrt_enable)
         {
             if(state.WB.Wrt_reg_addr==state.EX.Rs)
             state.EX.Read_data1 = state.WB.Wrt_data;

             if(state.WB.Wrt_reg_addr==state.EX.Rt)
             state.EX.Read_data2 = state.WB.Wrt_data;

         }

        if (!state.MEM.nop && state.MEM.wrt_enable)
            {
                if(state.MEM.Wrt_reg_addr==state.EX.Rs)
                state.EX.Read_data1 = state.MEM.ALUresult;

                if(state.MEM.Wrt_reg_addr==state.EX.Rt)
                state.EX.Read_data2 = state.MEM.ALUresult;
            }



		if(state.EX.is_I_type)
		{
			ALUin1 = state.EX.Read_data1;
			ALUin2 = signextend(state.EX.Imm);
        }
		else
		{
			ALUin1 = state.EX.Read_data1;
			ALUin2 = state.EX.Read_data2;
		}
		newState.MEM.ALUresult = bitset<32>((state.EX.alu_op)?ALUin1.to_ulong()+ALUin2.to_ulong():ALUin1.to_ulong()-ALUin2.to_ulong());
		newState.MEM.Store_data = state.EX.Read_data2;
		newState.MEM.Rs = state.EX.Rs;
		newState.MEM.Rt = state.EX.Rt;
		newState.MEM.Wrt_reg_addr =  state.EX.Wrt_reg_addr;
		newState.MEM.rd_mem =  state.EX.rd_mem;
		newState.MEM.wrt_mem =  state.EX.wrt_mem;
		newState.MEM.wrt_enable = state.EX.wrt_enable;

		newState.MEM.nop = state.EX.nop;
		}


        /* --------------------- ID stage --------------------- */
		/*r2 = M2(r1);*/
		if(state.ID.nop)
        {
            newState.EX.nop = true;
        }
        else
        {



			//    cout << "instruction number " << i+1 << endl;
        opcode = bitset<6> (shiftbits(state.ID.Instr, 26));
        RType = (opcode.to_ulong()==0)?1:0;
        IType = (opcode.to_ulong()!=0 && opcode.to_ulong()!=2)?1:0;
        JType = (opcode.to_ulong()==2)?1:0;
        IsBranch = (opcode.to_ulong()==4)?1:0;
        IsLoad = (opcode.to_ulong()==35)?1:0;
        IsStore = (opcode.to_ulong()==43)?1:0;
        WrtEnable = (IsStore.to_ulong() || IsBranch.to_ulong() || JType.to_ulong())?0:1;





        newState.EX.is_I_type = (IType.to_ulong())?true:false;
		newState.EX.rd_mem = (IsLoad.to_ulong())?true:false;
		newState.EX.wrt_mem = (IsStore.to_ulong())?true:false;
		newState.EX.wrt_enable = (WrtEnable.to_ulong())?true:false;

        funct = bitset<6> (shiftbits(state.ID.Instr, 0));
        newState.EX.Rs = bitset<5> (shiftbits(state.ID.Instr, 21));
        newState.EX.Rt = bitset<5> (shiftbits(state.ID.Instr, 16));
        newState.EX.Wrt_reg_addr =  (IType.to_ulong())? newState.EX.Rt : bitset<5> (shiftbits(state.ID.Instr, 11));
        newState.EX.alu_op = (IType.to_ulong())?true:(funct.to_ulong()==35)?false:true;
        newState.EX.Imm = bitset<16> (shiftbits(state.ID.Instr, 0));
        signext = signextend(newState.EX.Imm);

        braddr = bitset<32>(
            state.IF.PC.to_ulong()
            + (bitset<32>((bitset<30> (shiftbits(signext,0))).to_string<char,std::string::traits_type,std::string::allocator_type>()+"00")).to_ulong() );

		newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
		newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);

        if(state.EX.rd_mem)
        {
          if(state.EX.Rt == newState.EX.Rt || state.EX.Rt == newState.EX.Rs)

            {
             stall = true;

            }
        }




		if(IsBranch.to_ulong())
		{
			if(newState.EX.Read_data1 != newState.EX.Read_data2)
			{
				IsTaken = true;
				//newState.IF.PC = braddr;
                //newState.ID.nop = true;
			}
		}

		newState.EX.nop = state.ID.nop;

        }


		/* --------------------- IF stage --------------------- */
		/*r1 = getnew();*/

		if(state.IF.nop)
        {
            newState.ID.nop = true;
        }
        else
        {

        newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
        //cout<< state.ID.Instr<<"Clk:"<<clk<<"\n";


		newState.IF.PC = bitset<32> (state.IF.PC.to_ulong()+4);
        //cout<< newStateIF.PC;

        newState.ID.nop = state.IF.nop;
        newState.IF.nop = false;

        if(stall)
        {
            stall = false;
            newState.IF = state.IF;
            newState.ID = state.ID;
            newState.EX.nop = true;
        }

        if(IsTaken)
        {
            IsTaken = false;
            newState.IF.PC = braddr;
            newState.ID.nop = true;
            //cout<<"I was taken";
            //cout<<braddr.to_ulong();
        }

		if (state.ID.Instr.to_string<char,std::string::traits_type,std::string::allocator_type>()=="11111111111111111111111111111111")
        {
			//    myRF.OutputFile();
			newState.IF.nop = true;
			newState.ID.nop = true;
			newState.EX.nop = true;
        }

            //cout<<newState.IF.PC;
            //cout<<newState.ID.nop;


        }


        printState(newState,clk);
         //print states after executing cycle 0, cycle 1, cycle 2 ...
        state.EX.alu_op = newState.EX.alu_op;
        state.EX.Imm = newState.EX.Imm;
        state.EX.is_I_type = newState.EX.is_I_type;
        state.EX.rd_mem = newState.EX.rd_mem;
        state.EX.Read_data1 = newState.EX.Read_data1;
        state.EX.Read_data2 = newState.EX.Read_data2;
        state.EX.Rs = newState.EX.Rs;
        state.EX.Rt = newState.EX.Rt;
        state.EX.wrt_enable = newState.EX.wrt_enable;
        state.EX.wrt_mem = newState.EX.wrt_mem;
        state.EX.Wrt_reg_addr = newState.EX.Wrt_reg_addr;
        state.EX.nop = newState.EX.nop;


        state.WB.Rs = newState.WB.Rs;
        state.WB.Rt = newState.WB.Rt;
        state.WB.Wrt_data = newState.WB.Wrt_data;
        state.WB.wrt_enable = newState.WB.wrt_enable;
        state.WB.Wrt_reg_addr = newState.WB.Wrt_reg_addr;
        state.WB.nop = newState.WB.nop;


        state.MEM.ALUresult = newState.MEM.ALUresult;
        state.MEM.rd_mem = newState.MEM.rd_mem;
        state.MEM.Rs = newState.MEM.Rs;
        state.MEM.Rt = newState.MEM.Rt;
        state.MEM.Store_data = newState.MEM.Store_data;
        state.MEM.wrt_enable = newState.MEM.wrt_enable;
        state.MEM.wrt_mem = newState.MEM.wrt_mem;
        state.MEM.Wrt_reg_addr = newState.MEM.Wrt_reg_addr;
        state.MEM.nop = newState.MEM.nop;


        state.ID.Instr = newState.ID.Instr;
        state.ID.nop = newState.ID.nop;

        state.IF.nop = newState.IF.nop;
        state.IF.PC = newState.IF.PC;




        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            {
            break;
            }


         /*The end of the cycle and updates the current state with the values calculated in this cycle */
        clk++;
        //cout<<state.IF.PC<<"\n";


		}


    myRF.outputRF(); // dump RF;
	myDataMem.outputDataMem(); // dump data mem

	return 0;
}
