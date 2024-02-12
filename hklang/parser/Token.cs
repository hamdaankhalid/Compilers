using System.Text.Json.Serialization;

namespace parser;

public struct Token 
{
    [JsonPropertyName("token_type")]
	public string TypeOf { get; set; }

    [JsonPropertyName("value")]
	public string Value { get; set; }

    [JsonPropertyName("line")]
	public int Line { get; set; }

    [JsonPropertyName("column")]
	public int Column { get; set; }
}
