from init import GrammarProcessor
from calculate import CalculateSets
from table import LL1TableGenerator

class SyntaxAnalyzer:
    def __init__(self, grammar_processor, calculate_sets, ll1_table_generator):
        self.grammar_processor = grammar_processor
        self.calculate_sets = calculate_sets
        self.ll1_table_generator = ll1_table_generator
        self.tokens = []
        self.sym_stack = []
        self.type_sets = set()

    def get_tokens(self, file_name):
        self.tokens = []  #清空旧的 token
        with open(file_name, 'r') as file:
            for line in file:
                # 假设每行的格式为 'token <type,value>'
                parts = line.strip().split(' ')
                token = parts[0]
                type_value = parts[1].strip('<>').split(',')
                token_type = type_value[0]  # 获取类型
                self.type_sets.add(token_type)
                if token in self.grammar_processor.terminals:
                    self.tokens.append(token)
                elif token in self.grammar_processor.non_terminals:
                    self.tokens.append(token)
                else:
                    self.tokens.append(token_type)
                # self.tokens.append(token)
            self.tokens.append("EOF")

    def grammar_analysis(self, output_path):
        with open(output_path, 'w') as file:
            stack = ['#', self.grammar_processor.start]  # 初始化分析栈
            token_index = 0
            current_token = self.tokens[token_index]
            analysis_active = True
            action_count = 1  # 步骤编号

            while analysis_active:
                X = stack[-1]  # 获取栈顶符号

                if X == 'EOF':
                    if current_token == X:
                        file.write(f"{action_count}\tEOF#{current_token}\taccept\n")
                    else:
                        file.write(f"{action_count}\t{X}#{current_token}\terror\n")
                    break

                elif self.calculate_sets.is_terminal(X) or X == 'ε':
                    if X == current_token or X == 'ε':
                        action = "reduction" if X == 'ε' else "move"
                        file.write(f"{action_count}\t{X}#{current_token}\t{action}\n")
                        if X != 'ε':
                            token_index += 1
                            current_token = self.tokens[token_index] if token_index < len(self.tokens) else '#'
                        stack.pop()
                    else:
                        file.write(f"{action_count}\t{X}#{current_token}\terror\n")
                        break

                elif self.calculate_sets.is_non_terminal(X):
                    parsing_key = (X, current_token)
                    if parsing_key in self.ll1_table_generator.table and self.ll1_table_generator.table[parsing_key]:
                        production = self.ll1_table_generator.table[parsing_key][0].split()
                        stack.pop()
                        if production != ['ε']:
                            stack.extend(reversed(production))
                        file.write(f"{action_count}\t{X}#{current_token}\treduction\n")
                    else:
                        file.write(f"{action_count}\t{X}#{current_token}\terror\n")
                        break

                action_count += 1


# 使用示例
if __name__ == "__main__":
    grammar_processor = GrammarProcessor()
    grammar_processor.process_grammar_ref("grammar_ref.txt")
    calculate_sets = CalculateSets(grammar_processor)
    calculate_sets.calculate()
    ll1_table_generator = LL1TableGenerator(grammar_processor, calculate_sets)
    ll1_table_generator.compute_table()
    syntax_analyzer = SyntaxAnalyzer(grammar_processor, calculate_sets, ll1_table_generator)
    syntax_analyzer.get_tokens("./lex/lex1.txt")
    syntax_analyzer.grammar_analysis('./gra/gra1.txt')
    syntax_analyzer.get_tokens("./lex/lex2.txt")
    syntax_analyzer.grammar_analysis('./gra/gra2.txt')
    syntax_analyzer.get_tokens("./lex/lex3.txt")
    syntax_analyzer.grammar_analysis('./gra/gra3.txt')
    syntax_analyzer.get_tokens("./lex/lex4.txt")
    syntax_analyzer.grammar_analysis('./gra/gra4.txt')
    syntax_analyzer.get_tokens("./lex/lex5.txt")
    syntax_analyzer.grammar_analysis('./gra/gra5.txt')