from init import GrammarProcessor
from calculate import CalculateSets

class LL1TableGenerator:
    def __init__(self, grammar_processor, calculate_sets):
        self.grammar_processor = grammar_processor  
        self.calculate_sets = calculate_sets        
        self.table = {}  # 分析表：{('non_ter', 'ter'): ['prod']}  

    def compute_table(self):
        # 初始化分析表：每个非终结符 × 每个终结符（包括 #）→ 空列表
        for nt in self.grammar_processor.non_terminals:
            for t in self.grammar_processor.terminals.union({'#'}):
                self.table[(nt, t)] = []

        # 填表过程
        for nt in self.grammar_processor.non_terminals:
            for rule in self.grammar_processor.productions[nt]:
                symbols = rule.split()
                first_set = self.calculate_sets.compute_first(symbols)

                # 若 FIRST 集中含非空符号，则对每个终结符添加产生式
                for token in first_set:
                    if token != 'ε':
                        self.table[(nt, token)].append(rule)

                # 若 FIRST 集包含 ε，则对 FOLLOW 集中每个符号添加产生式
                if 'ε' in first_set:
                    for follower in self.calculate_sets.follow_sets[nt]:
                        self.table[(nt, follower)].append(rule)

    def print_table(self):
        with open('table.txt', 'w', encoding='utf-8') as file:
            file.write("LL(1) Analysis Table:\n")
            for nt in self.grammar_processor.non_terminals:
                for t in self.grammar_processor.terminals.union({'#'}):
                    entries = self.table[(nt, t)]
                    if entries:
                        row = f"({nt}, {t}) = {{ {nt} -> {' | '.join(entries)} }}\n"
                        file.write(row)


# 使用示例
if __name__ == "__main__":
    grammar_processor = GrammarProcessor()
    grammar_processor.process_grammar_ref("grammar_ref.txt")
    # grammar_processor.process_grammar("grammar.txt")
    calculate_sets = CalculateSets(grammar_processor)
    calculate_sets.calculate()
    ll1_table_generator = LL1TableGenerator(grammar_processor, calculate_sets)
    ll1_table_generator.compute_table()
    ll1_table_generator.print_table()
