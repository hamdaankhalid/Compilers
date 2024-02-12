using System.Text.Json;
using System.Collections.Generic;

namespace parser;


public class TokenBuffer
{
	private List<Token> _buf = new List<Token>();
	
	public void ReadInto(string line) {
		try
		{
			line = line.Trim();
			Console.WriteLine(line);
			Token token = JsonSerializer.Deserialize<Token>(line);
			this._buf.Add(token);
		} catch (JsonException ex)
		{
			Console.WriteLine($"Failed to serialize into token buf {line}. Exception {ex.Message}");
		}
	}

	public void Print() {
	}
}
