using System;

namespace parser;

class Program
{
    static void Main(string[] args)
    {
		Console.WriteLine("Constructing AST From Standard In");
		
		TokenBuffer buf = new TokenBuffer();

		string line;
		while ((line = Console.ReadLine()) != null) 
		{
			buf.ReadInto(line);
		}

		buf.Print();

		// create AST
    }
}
