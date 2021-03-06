/*
Copyright (c) 2012-2014 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
#pragma once

#include <memory>

#include "../server.h"
#include "strings.h"
//#include "../link.h"

// readonly
// to replace std::string
class Bytes{
	private:
		const char *data_;
		int size_;
	public:
		Bytes(){
			data_ = "";
			size_ = 0;
		}

		Bytes(void *data, int size){
			data_ = (char *)data;
			size_ = size;
		}

		Bytes(const char *data, int size){
			data_ = data;
			size_ = size;
		}

		Bytes(const std::string &str){
			data_ = str.data();
			size_ = (int)str.size();
		}

		Bytes(const char *str){
			data_ = str;
			size_ = (int)strlen(str);
		}

		const char* data() const{
			return data_;
		}

		bool empty() const{
			return size_ == 0;
		}

		int size() const{
			return size_;
		}

		void reclaim_data() {
			if (data_)	delete data_;
		}

		int compare(const Bytes &b) const{
			const int min_len = (size_ < b.size_) ? size_ : b.size_;
			int r = memcmp(data_, b.data_, min_len);
			if(r == 0){
				if (size_ < b.size_) r = -1;
				else if (size_ > b.size_) r = +1;
			}
			return r;
		}

		std::string String() const{
			return std::string(data_, size_);
		}

#if 0
		std::string *get_String() const{
			std::string str(data_, size_);
			return &str;
		}
#endif

		std::string *get_new_String() const{
			return new std::string(data_, size_);
		}

		int Int() const{
			return str_to_int(data_, size_);
		}

		int64_t Int64() const{
			return str_to_int64(data_, size_);
		}

		uint64_t Uint64() const{
			return str_to_uint64(data_, size_);
		}

		double Double() const{
			return str_to_double(data_, size_);
		}
};

inline
bool operator==(const Bytes &x, const Bytes &y){
	return ((x.size() == y.size()) &&
			(memcmp(x.data(), y.data(), x.size()) == 0));
}

inline
bool operator!=(const Bytes &x, const Bytes &y){
	return !(x == y);
}

inline
bool operator>(const Bytes &x, const Bytes &y){
	return x.compare(y) > 0;
}

inline
bool operator>=(const Bytes &x, const Bytes &y){
	return x.compare(y) >= 0;
}

inline
bool operator<(const Bytes &x, const Bytes &y){
	return x.compare(y) < 0;
}

inline
bool operator<=(const Bytes &x, const Bytes &y){
	return x.compare(y) <= 0;
}

class Link;

class Buffer{
	private:
		char *buf;
		char *data_;
		int size_;
		int last_sz;
		int total_;
		int origin_total;
	public:
		int32_t id;
		int32_t _count;
		Link *owner;  // yue
		bool fwd_to_replicas;
		uint served_replicas;


#ifdef USE_SMART_PTR
		std::shared_ptr<Buffer> peer;
#else
		Buffer *peer; // yue
#endif

		Link *head; //ali

		Buffer(int total);
		Buffer(char *ptr, int sz); // yue
		~Buffer();

		int total() const{ // 缓冲区大小
			return total_;
		}

		bool empty() const{
			return size_ == 0;
		}

		// 数据
		char* data() const{
			return data_;
		}

		/*
		char* buf() const{
			return buf;
		}
		*/

		int size() const{
			return size_;
		}

		char* slot() const{
			return data_ + size_;
		}

		int space() const{
			return total_ - (int)(data_ - buf) - size_;
		}

		void incr(int num){
			size_ += num;
		}

		void decr(int num){
			size_ -= num;
			data_ += num;
		}

		void remember_sz(int sz) { // yue
			last_sz = sz;
		}

		void reset_dataptr() { // yue
			size_ = last_sz;
			data_ = buf;
		}

		int size_diff() { // yue
			if(!last_sz >= size_)
				exit(1);
			return last_sz - size_;
		}

		// 保证不改变后半段的数据, 以便使已生成的 Bytes 不失效.
		void nice();
		// 扩大缓冲区
		int grow();
		// yue: reset the data_ pointer to buf and reset size_ to total
		void rewind(); 
		// yue: reset the data_ pointer to buf and reset size_ to sz 
		void rewind(int sz); 

		std::string stats() const;
		int read_record(Bytes *s);

		int append(char c);
		int append(const char *p);
		int append(const void *p, int size);
		int append(const Bytes &s);

		int append_record(const Bytes &s);
};


class Decoder{
private:
	const char *p;
	int size;
	Decoder(){}
public:
	Decoder(const char *p, int size){
		this->p = p;
		this->size = size;
	}
	int skip(int n){
		if(size < n){
			return -1;
		}
		p += n;
		size -= n;
		return n;
	}
	int read_int64(int64_t *ret){
		if(size_t(size) < sizeof(int64_t)){
			return -1;
		}
		if(ret){
			*ret = *(int64_t *)p;
		}
		p += sizeof(int64_t);
		size -= sizeof(int64_t);
		return sizeof(int64_t);
	}
	int read_uint64(uint64_t *ret){
		if(size_t(size) < sizeof(uint64_t)){
			return -1;
		}
		if(ret){
			*ret = *(uint64_t *)p;
		}
		p += sizeof(uint64_t);
		size -= sizeof(uint64_t);
		return sizeof(uint64_t);
	}
	int read_data(std::string *ret){
		int n = size;
		if(ret){
			ret->assign(p, size);
		}
		p += size;
		size = 0;
		return n;
	}
	int read_8_data(std::string *ret=NULL){
		if(size < 1){
			return -1;
		}
		int len = (uint8_t)p[0];
		p += 1;
		size -= 1;
		if(size < len){
			return -1;
		}
		if(ret){
			ret->assign(p, len);
		}
		p += len;
		size -= len;
		return 1 + len;
	}
};

