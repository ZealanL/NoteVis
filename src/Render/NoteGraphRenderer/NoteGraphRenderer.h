#pragma once
#include "../../Note/NoteGraph/NoteGraph.h"
#include "../../../Types/Vec.h"

// NoteGraphRender: Responsible for rendering the NoteGraph
namespace NoteGraphRenderer {
	void DrawNoteGraph(NoteGraph& graph, Area screenArea);
}