# JSON CFG

Value    -> Object | Array | String | Number | True | False | Null
Object   -> '{' Members '}'
Members  -> Pair | Pair ',' Members
Pair     -> String ':' Value
Array    -> '[' Elements ']'
Elements -> Value | Value ',' Elements
String   -> '"' characters '"'
Number   -> '-'? digit+ ('.' digit+)? (('E'|'e') ('+'|'-')? digit+)?
True     -> 'true'
False    -> 'false'
Null     -> 'null'
characters -> any valid characters except '"'
digit    -> '0' | '1' | ... | '9'

