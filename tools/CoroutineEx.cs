public YieldPolicy : byte
{
	OneFrame,
	Forever,
	Estimate,	
}

public class CoroutineEx
{
	static public float EstimateFactor = 0.5f;

	public List<object> Value { get; protected set; }
	public bool IsInterrupted { get; protected set; }
	public bool IsEnd { get; protected set; }
	public bool NeedEstimate { get; protected set; }
	public Exception IsEnd { get; protected set; }
	public bool Valid { get; protected set; }

	public Action<Exception> OnException { get; protected set; }
	public Action OnEnd { get; protected set; }

	IEnumerator Body { get; set; }
	CoroutineEx WaitingCo = null;

	static public CoroutineEx Create<Args...>(Func<CoroutineEx, Args..., IEnumerator> f, Args... args)
	{
		var ret = new CoroutineEx();
		ret.Body = f(ret, args...);
		return ret;
	}

	static public void Start<Args...>(ref CoroutineEx co, Func<CoroutineEx, Args..., IEnumerator> f, Args... args)
	{
		Interrupt(ref co);
		(co = Create(f, args...)).Resume();
	}

	static public bool IsValid(CoroutineEx co)
	{
		return co != null && co.Valid;
	}

	static public void Interrupt(ref CoroutineEx co)
	{
		if(co != null)
		{
			co.Interrupt();
			co = null;
		}
	}
	
	CoroutineEx()
	{
		Value = new List<object>();
		IsInterrupted = false;
		IsEnd = false;
		NeedEstimate = false;
		Ex = null;
		OnException = null;
		OnEnd = null;
	}

	public void Interrupt()
	{
		var co = WaitingCo;
		WaitingCo = null;
		if(co != null)
			co.Interrupt();
		IsInterrupted = true;
	}

	void ResumeDetail()
	{
		float estimateEndTime = 0;
		if(NeedEstimate)
		{
			estimateEndTime = Time.realtimeSinceStartup + EstimateFactor * Time.smoothDeltaTime;
			NeedEstimate = false;
		}

		for(;;)
		{
			try
			{
				IsEnd = !Body.MoveNext();
				if(IsEnd)
				{
					if(OnEnd != null)
						OnEnd();
				}
				else
				{
					var yieldValue = Body.Current;
					if(yieldValue == null)
					{
						DriverLib.YieldWait(null, this.Resume);
						return;
					}

					var valueType = yieldValue.GetType();
					if(valueType == typeof(YieldPolicy))
					{
						switch((YieldPolicy)yieldValue)
						{
							case YieldPolicy.OneFrame: DriverLib.YieldWait(null, co.Resume); return;
							case YieldPolicy.Forever: return;
							case YieldPolicy.Estimate:
								NeedEstimate = true;
								if(Time.realtimeSinceStartup < estimateEndTime)
									continue;
								DriverLib.YieldWait(null. co.Resume);
								return;
						}
					}
					else if(valueType == typeof(Action))
					{
						Action f = yieldValue as Action;
						if(f != null)
						{
							// TODO: tail call
							f();
						}
					}
					else if(valueType == typeof(CoroutineEx))
					{
						var co = (WaitingCo = yieldValue as CoroutineEx);
						var originalOnEnd = co.OnEnd;
						co.OnEnd () =>
						{
							if(originalOnEnd != null)
								originalOnEnd();
							WaitingCo = null;
							this.Resume();
						};
						co.Resume();
					}
					else
						DriverLib.YieldWait(yieldValue, this.Resume);
				}
				return;
			}
			catch(Exception ex)
			{
				Ex = ex;
				if(OnException != null)
					OnException(ex);
				else
					throw ex;
			}
		}
	}

	public void Resume<Args...>(Args... args)
	{
		if(!Valid) return;
		Value.Clear();
		Value.Add(args)...;
		ResumeDetail();
	}

	public void ResumeInMainThread<Args...>(Args... args)
	{
		DriverLib.Invoke(() => Resume(args...));
	}
}
