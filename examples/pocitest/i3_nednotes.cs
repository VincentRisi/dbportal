// Generated code from DBPortal - modify at source and not here

using System;
using System.Data;
using System.IO;
using System.Xml;
using System.Xml.Serialization;

namespace I3Server
{
  [XmlRoot] public struct NNGetCIFNoByHOGANCISNoInput : I3InStream
  {
    private string mHOGANCISNo;
    [XmlAttribute] public string HOGANCISNo {get {return mHOGANCISNo;} set {mHOGANCISNo=value;}}
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNGetCIFNoByHOGANCISNoOutput : I3InStream, I3OutStream
  {
    private int mRc;
    [XmlAttribute] public int Rc {get {return mRc;} set {mRc=value;}}
    private int mOciErr;
    [XmlAttribute] public int OciErr {get {return mOciErr;} set {mOciErr=value;}}
    private string mErrBuff;
    [XmlAttribute] public string ErrBuff {get {return mErrBuff;} set {mErrBuff=value;}}
    private string mCIF_No;
    [XmlAttribute] public string CIF_No {get {return mCIF_No;} set {mCIF_No=value;}}
    public void PutStream(MemoryStream memS)
    {
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      this = (NNGetCIFNoByHOGANCISNoOutput) xmlS.Deserialize(memS);
    }
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNGetSettlementAccountsInput : I3InStream
  {
    private string mCIF_No;
    [XmlAttribute] public string CIF_No {get {return mCIF_No;} set {mCIF_No=value;}}
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNGetSettlementAccountsOutput : I3InStream, I3OutStream
  {
    public struct ResultList
    {
      private string mAccount_No;
      [XmlAttribute] public string Account_No {get {return mAccount_No;} set {mAccount_No=value;}}
      private string mSWIFT_ID;
      [XmlAttribute] public string SWIFT_ID {get {return mSWIFT_ID;} set {mSWIFT_ID=value;}}
      private string mAcc_Seq_No;
      [XmlAttribute] public string Acc_Seq_No {get {return mAcc_Seq_No;} set {mAcc_Seq_No=value;}}
      private string mAccType_Code;
      [XmlAttribute] public string AccType_Code {get {return mAccType_Code;} set {mAccType_Code=value;}}
      private string mBranch_Code;
      [XmlAttribute] public string Branch_Code {get {return mBranch_Code;} set {mBranch_Code=value;}}
      private string mBranch_Clearing_No;
      [XmlAttribute] public string Branch_Clearing_No {get {return mBranch_Clearing_No;} set {mBranch_Clearing_No=value;}}
      private string mCCYCode;
      [XmlAttribute] public string CCYCode {get {return mCCYCode;} set {mCCYCode=value;}}
      private string mAcc_Full_Name;
      [XmlAttribute] public string Acc_Full_Name {get {return mAcc_Full_Name;} set {mAcc_Full_Name=value;}}
      private string mAcc_Short_Name;
      [XmlAttribute] public string Acc_Short_Name {get {return mAcc_Short_Name;} set {mAcc_Short_Name=value;}}
      private string mTax_Status_Code;
      [XmlAttribute] public string Tax_Status_Code {get {return mTax_Status_Code;} set {mTax_Status_Code=value;}}
      private string mAcc_Status_Code;
      [XmlAttribute] public string Acc_Status_Code {get {return mAcc_Status_Code;} set {mAcc_Status_Code=value;}}
      private string mResidency_Ind;
      [XmlAttribute] public string Residency_Ind {get {return mResidency_Ind;} set {mResidency_Ind=value;}}
      private string mMain_Acc_Ind;
      [XmlAttribute] public string Main_Acc_Ind {get {return mMain_Acc_Ind;} set {mMain_Acc_Ind=value;}}
      private string mAcc_Desc;
      [XmlAttribute] public string Acc_Desc {get {return mAcc_Desc;} set {mAcc_Desc=value;}}
      private string mVersion_No;
      [XmlAttribute] public string Version_No {get {return mVersion_No;} set {mVersion_No=value;}}
      private string mStatus;
      [XmlAttribute] public string Status {get {return mStatus;} set {mStatus=value;}}
      private string mUserId;
      [XmlAttribute] public string UserId {get {return mUserId;} set {mUserId=value;}}
      private string mTMstamp;
      [XmlAttribute] public string TMstamp {get {return mTMstamp;} set {mTMstamp=value;}}
      private string mSourceId;
      [XmlAttribute] public string SourceId {get {return mSourceId;} set {mSourceId=value;}}
    }
    private int mRc;
    [XmlAttribute] public int Rc {get {return mRc;} set {mRc=value;}}
    private int mOciErr;
    [XmlAttribute] public int OciErr {get {return mOciErr;} set {mOciErr=value;}}
    private string mErrBuff;
    [XmlAttribute] public string ErrBuff {get {return mErrBuff;} set {mErrBuff=value;}}
    private int mCount;
    [XmlAttribute] public int Count {get {return mCount;} set {mCount=value;}}
    [XmlElement] public ResultList[] List;
    public DataTable GetDataTable()
    {
      DataTable result = new DataTable();
      result.Columns.Add(new DataColumn("Account_No", typeof(String)));
      result.Columns.Add(new DataColumn("SWIFT_ID", typeof(String)));
      result.Columns.Add(new DataColumn("Acc_Seq_No", typeof(String)));
      result.Columns.Add(new DataColumn("AccType_Code", typeof(String)));
      result.Columns.Add(new DataColumn("Branch_Code", typeof(String)));
      result.Columns.Add(new DataColumn("Branch_Clearing_No", typeof(String)));
      result.Columns.Add(new DataColumn("CCYCode", typeof(String)));
      result.Columns.Add(new DataColumn("Acc_Full_Name", typeof(String)));
      result.Columns.Add(new DataColumn("Acc_Short_Name", typeof(String)));
      result.Columns.Add(new DataColumn("Tax_Status_Code", typeof(String)));
      result.Columns.Add(new DataColumn("Acc_Status_Code", typeof(String)));
      result.Columns.Add(new DataColumn("Residency_Ind", typeof(String)));
      result.Columns.Add(new DataColumn("Main_Acc_Ind", typeof(String)));
      result.Columns.Add(new DataColumn("Acc_Desc", typeof(String)));
      result.Columns.Add(new DataColumn("Version_No", typeof(String)));
      result.Columns.Add(new DataColumn("Status", typeof(String)));
      result.Columns.Add(new DataColumn("UserId", typeof(String)));
      result.Columns.Add(new DataColumn("TMstamp", typeof(String)));
      result.Columns.Add(new DataColumn("SourceId", typeof(String)));
      foreach (ResultList L in List)
      {
        DataRow dr = result.NewRow();
        dr[0] = L.Account_No;
        dr[1] = L.SWIFT_ID;
        dr[2] = L.Acc_Seq_No;
        dr[3] = L.AccType_Code;
        dr[4] = L.Branch_Code;
        dr[5] = L.Branch_Clearing_No;
        dr[6] = L.CCYCode;
        dr[7] = L.Acc_Full_Name;
        dr[8] = L.Acc_Short_Name;
        dr[9] = L.Tax_Status_Code;
        dr[10] = L.Acc_Status_Code;
        dr[11] = L.Residency_Ind;
        dr[12] = L.Main_Acc_Ind;
        dr[13] = L.Acc_Desc;
        dr[14] = L.Version_No;
        dr[15] = L.Status;
        dr[16] = L.UserId;
        dr[17] = L.TMstamp;
        dr[18] = L.SourceId;
        result.Rows.Add(dr);
      }
      return result;
    }
    public void PutStream(MemoryStream memS)
    {
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      this = (NNGetSettlementAccountsOutput) xmlS.Deserialize(memS);
    }
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNGetSettlementAccountStatusInput : I3InStream
  {
    private string mCIF_No;
    [XmlAttribute] public string CIF_No {get {return mCIF_No;} set {mCIF_No=value;}}
    private string mAccount_No;
    [XmlAttribute] public string Account_No {get {return mAccount_No;} set {mAccount_No=value;}}
    private string mBranch_Clearing_No;
    [XmlAttribute] public string Branch_Clearing_No {get {return mBranch_Clearing_No;} set {mBranch_Clearing_No=value;}}
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNGetSettlementAccountStatusOutput : I3InStream, I3OutStream
  {
    private int mRc;
    [XmlAttribute] public int Rc {get {return mRc;} set {mRc=value;}}
    private int mOciErr;
    [XmlAttribute] public int OciErr {get {return mOciErr;} set {mOciErr=value;}}
    private string mErrBuff;
    [XmlAttribute] public string ErrBuff {get {return mErrBuff;} set {mErrBuff=value;}}
    private string mAcc_Status_Code;
    [XmlAttribute] public string Acc_Status_Code {get {return mAcc_Status_Code;} set {mAcc_Status_Code=value;}}
    public void PutStream(MemoryStream memS)
    {
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      this = (NNGetSettlementAccountStatusOutput) xmlS.Deserialize(memS);
    }
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNGetContactInfoInput : I3InStream
  {
    private string mCIF_No;
    [XmlAttribute] public string CIF_No {get {return mCIF_No;} set {mCIF_No=value;}}
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNGetContactInfoOutput : I3InStream, I3OutStream
  {
    public struct ResultList
    {
      private string mContact_Purpose_Code;
      [XmlAttribute] public string Contact_Purpose_Code {get {return mContact_Purpose_Code;} set {mContact_Purpose_Code=value;}}
      private string mProduct_Code;
      [XmlAttribute] public string Product_Code {get {return mProduct_Code;} set {mProduct_Code=value;}}
      private string mSystem_Code;
      [XmlAttribute] public string System_Code {get {return mSystem_Code;} set {mSystem_Code=value;}}
      private string mContact_Name;
      [XmlAttribute] public string Contact_Name {get {return mContact_Name;} set {mContact_Name=value;}}
      private string mContact_Tel_No;
      [XmlAttribute] public string Contact_Tel_No {get {return mContact_Tel_No;} set {mContact_Tel_No=value;}}
      private string mDelivery_Method;
      [XmlAttribute] public string Delivery_Method {get {return mDelivery_Method;} set {mDelivery_Method=value;}}
      private string mDelivery_Detail;
      [XmlAttribute] public string Delivery_Detail {get {return mDelivery_Detail;} set {mDelivery_Detail=value;}}
      private string mStatus;
      [XmlAttribute] public string Status {get {return mStatus;} set {mStatus=value;}}
    }
    private int mRc;
    [XmlAttribute] public int Rc {get {return mRc;} set {mRc=value;}}
    private int mOciErr;
    [XmlAttribute] public int OciErr {get {return mOciErr;} set {mOciErr=value;}}
    private string mErrBuff;
    [XmlAttribute] public string ErrBuff {get {return mErrBuff;} set {mErrBuff=value;}}
    private int mCount;
    [XmlAttribute] public int Count {get {return mCount;} set {mCount=value;}}
    [XmlElement] public ResultList[] List;
    public DataTable GetDataTable()
    {
      DataTable result = new DataTable();
      result.Columns.Add(new DataColumn("Contact_Purpose_Code", typeof(String)));
      result.Columns.Add(new DataColumn("Product_Code", typeof(String)));
      result.Columns.Add(new DataColumn("System_Code", typeof(String)));
      result.Columns.Add(new DataColumn("Contact_Name", typeof(String)));
      result.Columns.Add(new DataColumn("Contact_Tel_No", typeof(String)));
      result.Columns.Add(new DataColumn("Delivery_Method", typeof(String)));
      result.Columns.Add(new DataColumn("Delivery_Detail", typeof(String)));
      result.Columns.Add(new DataColumn("Status", typeof(String)));
      foreach (ResultList L in List)
      {
        DataRow dr = result.NewRow();
        dr[0] = L.Contact_Purpose_Code;
        dr[1] = L.Product_Code;
        dr[2] = L.System_Code;
        dr[3] = L.Contact_Name;
        dr[4] = L.Contact_Tel_No;
        dr[5] = L.Delivery_Method;
        dr[6] = L.Delivery_Detail;
        dr[7] = L.Status;
        result.Rows.Add(dr);
      }
      return result;
    }
    public void PutStream(MemoryStream memS)
    {
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      this = (NNGetContactInfoOutput) xmlS.Deserialize(memS);
    }
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNCheckContactInfoInput : I3InStream
  {
    private string mCIF_No;
    [XmlAttribute] public string CIF_No {get {return mCIF_No;} set {mCIF_No=value;}}
    private string mContact_Purpose_Code;
    [XmlAttribute] public string Contact_Purpose_Code {get {return mContact_Purpose_Code;} set {mContact_Purpose_Code=value;}}
    private string mContact_Name;
    [XmlAttribute] public string Contact_Name {get {return mContact_Name;} set {mContact_Name=value;}}
    private string mContact_EMail_No;
    [XmlAttribute] public string Contact_EMail_No {get {return mContact_EMail_No;} set {mContact_EMail_No=value;}}
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
  [XmlRoot] public struct NNCheckContactInfoOutput : I3InStream, I3OutStream
  {
    private int mRc;
    [XmlAttribute] public int Rc {get {return mRc;} set {mRc=value;}}
    private int mOciErr;
    [XmlAttribute] public int OciErr {get {return mOciErr;} set {mOciErr=value;}}
    private string mErrBuff;
    [XmlAttribute] public string ErrBuff {get {return mErrBuff;} set {mErrBuff=value;}}
    private string mStatus;
    [XmlAttribute] public string Status {get {return mStatus;} set {mStatus=value;}}
    public void PutStream(MemoryStream memS)
    {
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      this = (NNCheckContactInfoOutput) xmlS.Deserialize(memS);
    }
    public MemoryStream GetStream()
    {
      MemoryStream memS = new MemoryStream();
      XmlSerializer xmlS = new XmlSerializer(this.GetType());
      xmlS.Serialize(memS, this);
      return memS;
    }
  }
}
