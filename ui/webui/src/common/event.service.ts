import { Injectable } from '@angular/core';
import { BehaviorSubject, Subscription, filter, map } from 'rxjs';
import { SubSink } from 'subsink';
import { IEventData } from './common';

@Injectable({
  providedIn: 'root'
})
export class EventService {

  constructor() { }
  private subsink = new SubSink();
  private evtData:IEventData = <IEventData>{id: "none", document: ""};
  private bs$: BehaviorSubject<{id:string, document:string}> = new BehaviorSubject<IEventData>(this.evtData);


  /**
   * This function publishes the Event to subscribed component
   * @param event 
   */
  public publish(event: {id:string, document:string}) {
    this.evtData = event;
    this.bs$.next(event);
  }

  /**
   * Publish compelte event to Observer.
   */
  public complete() {
    this.bs$.complete();
  }

  /**
   * Publish Error to observer.
   * @param event 
   */
  public error(event: {id:string, document:string}) {
    this.bs$.error(event);
  }

  private defaultAction = (id:string, document: string) => { return({id, document});}

  /**
   * 
   * @param evt 
   * @param eventHandler 
   * @returns 
   */
  public subscribe(event: string, eventHandler:any) {
    this.subsink.add(this.bs$.pipe(filter((evt: {id:string, document:string}) => evt.id == event), map((evt: {id:string, document:string}) => evt)).subscribe(
      (e: {id:string, document:string}) => {eventHandler(e.id, e.document);},
      (error) => {},
      () => {}
    ));
  }

  public on(event: {id:string, document:string}, eventHandler:any): Subscription {
    return(this.bs$.pipe(filter((evt: {id:string, document:string}) => evt.id == event.id), map((evt: {id:string, document:string}) => evt)).subscribe(eventHandler));
  }

  ngOnDestroy(): void {
      this.subsink.unsubscribe();
  }

  /*
  public on(event: EventsService, action:any): Subscription {
    return(this.bs$.pipe(filter((e:EmitEvent) => e.name == event),map((e:emitEvent) => e.value)).subscribe(action));
  }*/

}
