#include <bits/stdc++.h>

using namespace std;

#define HEX uppercase << hex

string format_number(int num, int width, char pad = '0')
{
    stringstream temp;
    temp << std::hex << std::uppercase << std::setfill(pad) << std::setw(width) << num;
    return temp.str();
}

bool isHexadecimal(string s) {
   int n = s.size();

   for (int i = 0; i < n; i++) {
      if (!isxdigit(s[i])) { // Check if the character is a valid hexadecimal digit
         return false;
      }
   }

   return true;
}

string inputFile = "assemblerOutput.txt", outputFile = "loaderOutput.txt";
int progaddr, csaddr, execaddr, cslth, endaddr;
string line;
unordered_map<string, int> estab;
unordered_map<string, int> lens = { // Map to get length of instructions from opcodes
    {"18", 3},
    {"58", 3},
    {"90", 2},
    {"40", 3},
    {"B4", 2},
    {"28", 3},
    {"88", 3},
    {"A0", 2},
    {"24", 3},
    {"64", 3},
    {"9C", 2},
    {"C4", 1},
    {"C0", 1},
    {"F4", 1},
    {"3C", 3},
    {"30", 3},
    {"34", 3},
    {"38", 3},
    {"48", 3},
    {"00", 3},
    {"68", 3},
    {"50", 3},
    {"70", 3},
    {"08", 3},
    {"6C", 3},
    {"74", 3},
    {"04", 3},
    {"D0", 3},
    {"20", 3},
    {"60", 3},
    {"98", 2},
    {"C8", 1},
    {"44", 3},
    {"D8", 3},
    {"AC", 2},
    {"4C", 3},
    {"A4", 2},
    {"A8", 2},
    {"F0", 1},
    {"EC", 3},
    {"0C", 3},
    {"78", 3},
    {"54", 3},
    {"80", 3},
    {"D4", 3},
    {"14", 3},
    {"7C", 3},
    {"E8", 3},
    {"84", 3},
    {"10", 3},
    {"1C", 3},
    {"5C", 3},
    {"94", 2},
    {"B0", 2},
    {"E0", 3},
    {"F8", 1},
    {"2C", 3},
    {"B8", 2},
    {"DC", 3}};

vector<pair<int, int>> textRecords;

bool errorFlag = false;
void setErrorFlag()
{
    cout << "ERROR" << '\n';
    errorFlag = true;
}

string trimWhitespaces(string s)
{ // Trims whitespaces from string
    while (s.size() && (s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    if (s.size() == 0)
        return s;
    int mi = s.size();
    for (int i = 0; i < s.size(); i++)
        if (s[i] != ' ' && s[i] != '\t')
        {
            mi = i;
            break;
        }
    s = s.substr(mi);
    return s;
}

long hexToInt(string s)
{ // Helper function to convert HEX to INT
    long temp;
    stringstream ss;
    ss << HEX << s;
    ss >> temp;
    return temp;
}

string intToHex(long n)
{ // Helper function to convert INT to HEX
    stringstream ss;
    ss << HEX << n;
    return ss.str();
}

string getHex2Comp(int num, int length)
{ // Get 2s complement form of num
    if (num >= 0)
    {
        return intToHex(num);
    }
    else
    {
        int comp = (1 << length) + num;
        return intToHex(comp);
    }
}

string padZeroesToLeft(string s, int sz = 6)
{ // Pads string with zeroes in the beginning
    string temp;
    for (int i = 0; i < (sz - (int)s.size()); i++)
        temp += '0';
    return temp + s;
}

struct HeaderRecord
{ // Struct to parse Header records
    string secName;
    int startingAddress;
    int length;
    HeaderRecord(string &line)
    {
        secName = trimWhitespaces(line.substr(1, 6));
        startingAddress = hexToInt(trimWhitespaces(line.substr(7, 6)));
        length = hexToInt(trimWhitespaces(line.substr(13, 6)));
    }
};

struct TextRecord
{ // Struct to parse text records and load to mmap
    int startingAddress;
    int length;
    string instructions;
    TextRecord(string &line)
    {
        startingAddress = hexToInt(trimWhitespaces(line.substr(1, 6)));
        length = hexToInt(trimWhitespaces(line.substr(7, 2)));
        instructions = line.substr(9);
        textRecords.push_back({(startingAddress + csaddr) * 2, length * 2});
    }

    void loadToMem(char mmap[])
    { // Loads the object code at the address in mmap
        int sz = instructions.size();
        strncpy(mmap + (csaddr + startingAddress) * 2, instructions.c_str(), sz);
    }
};

struct DefineRecord
{ // Struct to parse Define records
    vector<pair<string, int>> defSyms;
    DefineRecord(string &line)
    {
        int i = 1;
        string symbol;
        int address;
        while (i < line.size())
        {
            symbol = trimWhitespaces(line.substr(i, 6));
            address = hexToInt(trimWhitespaces(line.substr(i + 6, 6)));
            i += 12;
            defSyms.push_back({symbol, address});
        }
    }
};

struct ModificationRecord
{ // Struct to parse M records and make updates in mmap
    int address, length;
    char flag;
    string extSym;
    ModificationRecord(string &line)
    {
        address = hexToInt(trimWhitespaces(line.substr(1, 6)));
        length = hexToInt(trimWhitespaces(line.substr(7, 2)));
        flag = line[9];
        extSym = trimWhitespaces(line.substr(10, 6));
    }

    void updateVal(char mmap[])
    { // Updates value at address in mmap
        if (flag == '+')
        {
            addDelta(estab[extSym], mmap);
        }
        else
        {
            addDelta(-estab[extSym], mmap);
        }
    }

    void addDelta(int delta, char mmap[])
    { // Adds delta at the address in mmap
        char buf[length + 1];
        int effAddress = (csaddr + address) * 2 + length % 2;
        strncpy(buf, mmap + effAddress, length); // Get bytes at address
        buf[length] = '\0';
        string s(buf);
        int val = hexToInt(s);
        val += delta; // Add delta to it
        string temp = getHex2Comp(val, length * 4);
        temp = padZeroesToLeft(temp, length);

        strncpy(mmap + effAddress, temp.c_str(), length); // Write the bytes back in mmap
    }
};

struct EndRecord
{ // struct to parse EndRecord
    int startingAddress;
    EndRecord(string &line)
    {
        if (line.size() == 1)
            return;
        startingAddress = hexToInt(trimWhitespaces(line.substr(1, 6)));
    }
};

int adl = 6, ll = 10, ocl = 10, opl = 30; // Length of diff parts of input files
                                          // Used in parsing
struct InstructionInter
{ // Struct to hold intermediate instructions
    string address, label, opcode, operand;
    int type;
    bool empty;
    InstructionInter(string &line)
    {
        if (line.size() == 0)
        { // Checks if empty line
            empty = true;
            return;
        }
        address = line.substr(0, adl);
        address = trimWhitespaces(address);
        line = line.substr(adl);
        label = line.substr(0, ll);
        label = trimWhitespaces(label); // Parse input line to initialize various variables
        opcode = line.substr(ll, ocl);
        opcode = trimWhitespaces(opcode);
        operand = line.substr(ll + ocl, opl);
        operand = trimWhitespaces(operand);

        if (opcode[0] == '+')
        { // Check extended format instruction
            opcode = opcode.substr(1);
        }
        empty = false;
    }
};

unordered_map<int, int> constAddresses;  // Addresses of constants from Intermediate file
unordered_map<string, int> begAddresses; // Beginning addresses of sections
void checkConstants()
{
    ifstream fin("intermediate.txt");
    string curSec;
    while (!fin.eof())
    { // Read line-by-line and set beginning addresses with length in map
        getline(fin, line);
        if (line.size() == 0)
            break;
        if (line[0] == '.')
        { // It is a comment, so we ignore it
            continue;
        }
        InstructionInter inst(line);
        if (inst.opcode == "START" || inst.opcode == "CSECT")
            curSec = inst.label;
        else if (inst.opcode == "WORD")
            constAddresses[hexToInt(inst.address) + begAddresses[curSec]] = 3;
        else if (inst.opcode == "BYTE")
            constAddresses[hexToInt(inst.address) + begAddresses[curSec]] = 1;
        else if (inst.label == "*")
            constAddresses[hexToInt(inst.address) + begAddresses[curSec]] = inst.operand.size() / 2;
    }
}

void writeToStream(ofstream &fout1, ofstream &fout, char mmap[])
{ // Function to write the mmap in the output file

    checkConstants();

    // writing to memory.txt file
    string st(mmap);
    int i = (progaddr/ 16) * 16;
    int n = (endaddr/ 16) * 8;

    int it = i*2;
    while (i < n)
    {
        fout1 << format_number(i, 4) << ' ';

        for (int j = 0; j < 4; j++)
        {
            fout1 << st.substr(it, 8);
            it += 8;
            fout1 << ' ';
            i += 4;
        }
            fout1 << '\n';
        }

    // writing to loaderOutput.txt file
    for (int j = 0; j < textRecords.size(); j++)
    {
        auto textRecord = textRecords[j];
        int address = textRecord.first;
        int length = textRecord.second;
        int endadd = address + textRecord.second; // Calculating the end address from start address and length
        char buf[length + 1];                     // Buf for storing string
        strncpy(buf, mmap + address, length);
        buf[length] = '\0';
        string s(buf);

        int i = 0;
        while (i < s.size())
        { // Iterating byte-by-byte
            int curAddress = (i + address) / 2;
            ;
            int len;
            if (constAddresses.count(curAddress))
            { // Checking if constant or Instruction
                len = constAddresses[curAddress];
            }
            else
            {
                int fb = hexToInt(s.substr(i, 2));
                string temp = padZeroesToLeft(intToHex((fb >> 2) << 2), 2);
                len = lens[temp]; // Getting length from map lens
                if (len == 3)
                {
                    int flags = hexToInt(string(1, s.substr(i, 3)[2]));
                    if (flags & 1)
                        len = 4;
                }
            }
            fout << padZeroesToLeft(intToHex(curAddress)) << " " << s.substr(i, len * 2);
            i += (len * 2);
            if (j + 1 < textRecords.size() || i < s.size())
                fout << endl;
        }
    }
}

bool isStreamEmpty(std::ifstream& fileStream) {
    return fileStream.peek() == std::ifstream::traits_type::eof();
}


int main(int argc, char *argv[])
{

    ifstream fin(inputFile);   // Creating i/o streams for input/output files
    ofstream fout(outputFile); // Creating i/o streams for input/output files
    ofstream fout1("memory.txt");

    // handling the error of empty file input
    if (isStreamEmpty(fin))
    {
        cout << "Error: Empty file!!" << endl;
        return 0;
    }
   
    /**************** PASS 1 STARTS ****************/
    cout<<"Enter program starting address"<<endl;
    string saddr;
    cin>>saddr;

    //checking if input starting address is valid or not
    if(!isHexadecimal(saddr)){
        cout<<"Run program again and Enter a valid starting address in hexadecimal notation"<<endl;
        return 0;
    }
    long x=hexToInt(saddr);

    csaddr = progaddr= x;
    while (!fin.eof())
    {
        getline(fin, line);
        HeaderRecord header(line); // Parse and hold Header line
        cslth = header.length;
        if (estab.count(header.secName))
        { // checking if secName in estab
            setErrorFlag();
        }
        else
        {
            estab[header.secName] = csaddr;
        }

        getline(fin, line);
        while (line[0] != 'E')
        {
            if (line[0] == 'D')
            {
                DefineRecord defRecord(line); // Parsing and storing define record
                for (auto u : defRecord.defSyms)
                {
                    if (estab.count(u.first))
                    { // Checking if symbol already in estab
                        setErrorFlag();
                    }
                    else
                    {
                        estab[u.first] = u.second + csaddr;
                    }
                }
            }
            getline(fin, line); // Reading next line
        }

        if (!fin.eof())
            getline(fin, line);
        csaddr += cslth; // Adding section length to csaddr
    }
    /**************** PASS 1 ENDS ****************/
    cout << "PASS 1 completed.\n";

    fin.clear();
    fin.seekg(0); // Go back to beginning of input file

    endaddr = ((csaddr) * 2 + 31) / 32 * 32; // Calcuting end address for output file

    char mmap[1 << 18]; // mmap is the helper array which is used to load the program instead of actual memory
    for (int i = 0; i < (1 << 18); i++)
        mmap[i] = '.'; // Initializing mmap

    /**************** PASS 2 STARTS ****************/
    execaddr = csaddr = progaddr;
    while (!fin.eof())
    {

        getline(fin, line); // Reading line from input file
        HeaderRecord header(line);
        cslth = header.length;
        begAddresses[header.secName] = csaddr;
        getline(fin, line);
        while (line[0] != 'E')
        {
            if (line[0] == 'T')
            {
                TextRecord tRecord(line); // Parsing and storing text record
                tRecord.loadToMem(mmap);  // Load the object code in mmap
            }
            else if (line[0] == 'M')
            {
                ModificationRecord mRecord(line);
                if (estab.count(mRecord.extSym))
                {                            // Is estab has the extsymbol
                    mRecord.updateVal(mmap); // Update the value at the adress
                }
                else
                {
                    setErrorFlag();
                }
            }
            getline(fin, line); // Read next line
        }
        EndRecord eRecord(line);
        if (eRecord.startingAddress)
        {
            execaddr = csaddr + eRecord.startingAddress; // If starting Address is given, set execaddr to it
        }
        if (!fin.eof())
            getline(fin, line);
        csaddr += cslth; // Adding section length to csaddr
    }

    fin.close();

    /**************** PASS 2 ENDS ****************/
    cout << "PASS 2 completed.\n";
    writeToStream(fout1, fout, mmap);
    fout.close();
    fout1.close();
    if (errorFlag)
    {
        cout << "ERROR" << endl;
    }
    else
    {
        cout << "See " << outputFile << " and memory.txt files for the output" << endl;
    }
}