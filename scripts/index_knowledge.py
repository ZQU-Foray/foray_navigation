#!/usr/bin/env python3
"""foray_sentry_nav 项目知识索引脚本（Vector Backend Adapter）。

用法:
    .venv/bin/python scripts/index_knowledge.py                # 首次全量索引
    .venv/bin/python scripts/index_knowledge.py --query "建图"  # 查询验证

后端:    chromadb (PersistentClient, 本地持久化到 .vector-index/)
嵌入:    纯 numpy TF-IDF（字符 n-gram），无需下载模型，兼容只读 HOME 环境
"""

import argparse
import math
import pathlib
import sys

import numpy as np
import chromadb
from chromadb.api.types import Documents, EmbeddingFunction, Embeddings

INDEX_DIR = pathlib.Path(__file__).resolve().parent.parent / ".vector-index"
COLLECTION = "knowledge"
NGRAM = 3
DIM = 1024

# 索引对象: (仓库相对路径, 说明)
DOCS = [
    ("readme.md", "项目需求"),
    ("AGENTS.md", "AI 开发规范"),
    ("plan.md", "开发计划与进度"),
    ("tree.md", "目录结构"),
    ("decision.md", "决策日志"),
    ("docs/nav_stack_architecture_design.md", "架构设计"),
    ("docs/knowledge/folder_summary.md", "目录职责摘要"),
    ("docs/knowledge/file_summary.md", "文件职责摘要"),
]


def tokenize(text: str) -> list[str]:
    """字符 n-gram 词条（对小文档稳定、跨语言友好）。"""
    t = text.lower()
    return [t[i : i + NGRAM] for i in range(max(0, len(t) - NGRAM + 1))]


class TfidfEmbedding(EmbeddingFunction):
    """TF-IDF 加权 n-gram 嵌入（余弦相似度语义检索）。"""

    def __init__(self) -> None:
        self.idf: dict[str, float] = {}

    def fit(self, corpus: list[str]) -> "TfidfEmbedding":
        n_docs = len(corpus)
        df: dict[str, int] = {}
        for text in corpus:
            for gram in set(tokenize(text)):
                df[gram] = df.get(gram, 0) + 1
        self.idf = {g: math.log((1 + n_docs) / (1 + c)) + 1 for g, c in df.items()}
        return self

    def _vec(self, text: str) -> list[float]:
        v = np.zeros(DIM, dtype=np.float64)
        tf: dict[str, int] = {}
        for gram in tokenize(text):
            tf[gram] = tf.get(gram, 0) + 1
        for gram, count in tf.items():
            if gram in self.idf:
                v[hash(gram) % DIM] += count * self.idf[gram]
        norm = np.linalg.norm(v)
        if norm > 0:
            v /= norm
        return v.tolist()

    def __call__(self, input: Documents) -> Embeddings:
        return [self._vec(t) for t in input]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--query", help="查询验证关键词")
    args = ap.parse_args()

    root = INDEX_DIR.parent
    corpus: list[str] = []
    paths: list[str] = []
    for rel, _ in DOCS:
        p = root / rel
        if p.exists():
            corpus.append(p.read_text(encoding="utf-8"))
            paths.append(rel)

    ef = TfidfEmbedding().fit(corpus)
    client = chromadb.PersistentClient(path=str(INDEX_DIR))
    col = client.get_or_create_collection(COLLECTION, embedding_function=ef)

    if args.query:
        res = col.query(query_texts=[args.query], n_results=3)
        print(f"查询: {args.query}")
        for doc_id, dist in zip(res["ids"][0], res["distances"][0]):
            print(f"  - {doc_id}  (distance={dist:.3f})")
        return 0

    col.upsert(
        ids=paths,
        documents=corpus,
        metadatas=[{"file": p} for p in paths],
    )
    print(f"全量索引完成: {col.count()} 个文档 -> {INDEX_DIR}")

    # write -> query -> verify
    res = col.query(query_texts=["建图模式"], n_results=2)
    print("验证查询 '建图模式' 命中:", res["ids"][0])
    return 0


if __name__ == "__main__":
    sys.exit(main())
