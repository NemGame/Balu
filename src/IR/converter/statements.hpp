#pragma once

namespace IR::converter {
    Node* ConvertIfStmt(const ast::IfStmt& stmt) {
        bool negated = dynamic_cast<ast::UnaryExpr*>(stmt.Condition) && 
                       dynamic_cast<ast::UnaryExpr*>(stmt.Condition)->Operator.kind == lexer::NOT;
        Instruction ifInstruction(negated ? Opcode::JUMP_IF_FALSE : Opcode::JUMP_IF_TRUE);
    }
}