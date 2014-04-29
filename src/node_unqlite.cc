/*!
 * node-unqlite
 * Copyright(c) 2013 Hideaki Ohno <hide.o.j55{at}gmail.com>
 * MIT Licensed
 */
#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include "node_unqlite.h"
#include <iostream>

using namespace v8;
using namespace node;

Persistent<FunctionTemplate> NodeUnQLite::constructor_template;

void NodeUnQLite::Init(Handle<Object> exports) {
    NanScope();

    Local < FunctionTemplate > t = FunctionTemplate::New(NodeUnQLite::New);
    NanAssignPersistent(v8::FunctionTemplate, constructor_template, t);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->SetClassName(NanSymbol("Database"));

    NODE_SET_PROTOTYPE_METHOD(t, "open", Open);
    NODE_SET_PROTOTYPE_METHOD(t, "fetch", FetchKV);
    NODE_SET_PROTOTYPE_METHOD(t, "store", StoreKV);
    NODE_SET_PROTOTYPE_METHOD(t, "append", AppendKV);
    NODE_SET_PROTOTYPE_METHOD(t, "delete", DeleteKV);

    exports->Set(NanSymbol("Database"), t->GetFunction());
}

NodeUnQLite::NodeUnQLite() : db_(NULL), open_(false) {
}

NodeUnQLite::~NodeUnQLite() {
    if (db_ != NULL) {
        unqlite_close(db_);
        db_ = NULL;
    }
}

NAN_METHOD(NodeUnQLite::New) {
    NanScope();

    REQ_STR_ARG(0)
    std::string filename = *String::Utf8Value(args[0]->ToString());

    int pos = 1;

    int mode;
    if (args.Length() >= pos && args[pos]->IsInt32()) {
        mode = args[pos++]->Int32Value();
    } else {
        mode = UNQLITE_OPEN_CREATE;
    }

    Local < Function > callback;
    if (args.Length() >= pos && args[pos]->IsFunction()) {
        callback = Local < Function > ::Cast(args[pos++]);
    }

    NodeUnQLite* uql = new NodeUnQLite();
    uql->Wrap(args.Holder());
    args.This()->Set(NanSymbol("filename"), args[0]->ToString(), ReadOnly);

    NanReturnValue(args.Holder());
}

NAN_METHOD(NodeUnQLite::Open) {
    NanScope();

    int pos = 0;

    int mode;
    if (args.Length() >= pos && args[pos]->IsInt32()) {
        mode = args[pos++]->Int32Value();
    } else {
        mode = UNQLITE_OPEN_CREATE;
    }

    REQ_FUN_ARG(pos, cb);

    NodeUnQLite* uql = Unwrap<NodeUnQLite>(args.Holder());
    args.This()->Set(String::NewSymbol("mode"), Integer::New(mode), ReadOnly);
    std::string filename = *String::Utf8Value(args.This()->Get(NanSymbol("filename")));

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new NodeUnQLite::OpenWorker(callback, uql, filename, mode));
    NanReturnUndefined();
}


NAN_METHOD(NodeUnQLite::FetchKV) {
    NanScope();

    REQ_STR_ARG(0)
    std::string key = *String::Utf8Value(args[0]->ToString());

    REQ_FUN_ARG(1, cb);

    NodeUnQLite* uql = Unwrap<NodeUnQLite>(args.Holder());

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new NodeUnQLite::AccessWorker(callback, uql, T_UNQLITE_FETCH, key));

    NanReturnUndefined();
}

NAN_METHOD(NodeUnQLite::StoreKV) {
    NanScope();
    REQ_STR_ARG(0)
    std::string key = *String::Utf8Value(args[0]->ToString());
    REQ_STR_ARG(1)
    std::string value = *String::Utf8Value(args[1]->ToString());

    REQ_FUN_ARG(2, cb);

    NodeUnQLite* uql = Unwrap<NodeUnQLite>(args.Holder());

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new NodeUnQLite::AccessWorker(callback, uql, T_UNQLITE_STORE, key, value));

    NanReturnUndefined();
}


NAN_METHOD(NodeUnQLite::AppendKV) {
    NanScope();

    REQ_STR_ARG(0)
    std::string key = *String::Utf8Value(args[0]->ToString());
    REQ_STR_ARG(1)
    std::string value = *String::Utf8Value(args[1]->ToString());

    REQ_FUN_ARG(2, cb);

    NodeUnQLite* uql = Unwrap<NodeUnQLite>(args.Holder());

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new NodeUnQLite::AccessWorker(callback, uql, T_UNQLITE_APPEND, key, value));

    NanReturnUndefined();
}

NAN_METHOD(NodeUnQLite::DeleteKV) {
    NanScope();

    REQ_STR_ARG(0)
    std::string key = *String::Utf8Value(args[0]->ToString());

    REQ_FUN_ARG(1, cb);

    NodeUnQLite* uql = Unwrap<NodeUnQLite>(args.Holder());

    NanCallback *callback = new NanCallback(cb);
    NanAsyncQueueWorker(new NodeUnQLite::AccessWorker(callback, uql, T_UNQLITE_DELETE, key));

    NanReturnUndefined();
}

void Initialize(Handle<Object> exports) {
    NodeUnQLite::Init(exports);

    // define constants
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OK, OK);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_NOMEM, NOMEM);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_ABORT, ABORT);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_IOERR, IOERR);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_CORRUPT, CORRUPT);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_LOCKED, LOCKED);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_BUSY, BUSY);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_DONE, DONE);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_PERM, PERM);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_NOTIMPLEMENTED, NOTIMPLEMENTED);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_NOTFOUND, NOTFOUND);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_NOOP, NOOP);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_INVALID, INVALID);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_EOF, EOF);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_UNKNOWN, UNKNOWN);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_LIMIT, LIMIT);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_EXISTS, EXISTS);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_EMPTY, EMPTY);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_COMPILE_ERR, COMPILE_ERR);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_VM_ERR, VM_ERR);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_FULL, FULL);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_CANTOPEN, CANTOPEN);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_READ_ONLY, READ_ONLY);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_LOCKERR, LOCKERR);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_CURSOR_MATCH_EXACT, CURSOR_MATCH_EXACT);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_CURSOR_MATCH_LE, CURSOR_MATCH_LE);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_CURSOR_MATCH_GE, CURSOR_MATCH_GE);

    // mode
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_CREATE, OPEN_CREATE);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_READWRITE, OPEN_READWRITE);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_READONLY, OPEN_READONLY);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_MMAP, OPEN_MMAP);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_EXCLUSIVE, OPEN_EXCLUSIVE);  // VFS only
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_TEMP_DB, OPEN_TEMP_DB);      // VFS only
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_IN_MEMORY, OPEN_IN_MEMORY);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_OMIT_JOURNALING, OPEN_OMIT_JOURNALING);
    DEFINE_CONSTANT_INTEGER(exports, UNQLITE_OPEN_NOMUTEX, OPEN_NOMUTEX);
}

NODE_MODULE(node_unqlite, Initialize)
