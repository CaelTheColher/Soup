#pragma once

#include "fwd.hpp"

#include <vector>

#include "Op.hpp"

namespace soup
{
	class ParserState
	{
	public:
		const LangDesc* ld;
		astBlock* b;
		std::vector<UniquePtr<astNode>>::iterator i;
		Op op;

		void setOp(int type);
		void consumeLefthandValue();
		void consumeRighthandValue();

		void pushArg(Mixed&& val);
		void pushArgNode(UniquePtr<astNode>&& node);
		void setArgs(std::vector<UniquePtr<astNode>>&& args);

		void pushLefthand(Mixed&& val);
		void pushLefthand(Lexeme&& l);
		void pushLefthandNode(UniquePtr<astNode>&& node);
		UniquePtr<astNode> popLefthand();
		[[nodiscard]] astNode* peekRighthand() const;
		UniquePtr<astNode> popRighthand();
		UniquePtr<astBlock> collapseRighthandBlock(const char* end_token); // end_token must've been registered with addToken

		[[nodiscard]] const Token& getToken() const;

	private:
		void checkRighthand() const;
		void ensureValidIterator() const;
	};
}
