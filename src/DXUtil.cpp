namespace Memory 
{
	template<class T> void Release(T&t)
	{
		if(t)
		{
			t->Release();
			t = nullptr;
		}
	}

	template<class T> void Delete(T&t)
	{
		if(t)
		{
			delete t;
			t = nullptr;
		}
	}

	template<class T> void DeleteArr(T&t)
	{
		if(t)
		{
			delete[] t;
			t = nullptr;
		}
	}
}